diff --git a/board/p2041/inc/bsp_comapi.h b/board/p2041/inc/bsp_comapi.h
index f0e3da0..c661194 100644
--- a/board/p2041/inc/bsp_comapi.h
+++ b/board/p2041/inc/bsp_comapi.h
@@ -5,11 +5,12 @@
 #define RLIMIT_MSGQUEUE		12	/* maximum bytes in POSIX mqueues */
 #define RLIM_INFINITY		0x7fffffffUL
 //#define SRIO_SHARE_ALIGN              (0x8000000)
+/*
 struct rlimit {
 	unsigned long	rlim_cur;
 	unsigned long	rlim_max;
 };
-
+*/
 void bsp_sys_msdelay(ULONG dwTimeOut);
 void bsp_sys_usdelay(ULONG dwTimeOut);
 
diff --git a/board/p2041/src/bsp_comapi.c b/board/p2041/src/bsp_comapi.c
index 3af7781..1c023e7 100644
--- a/board/p2041/src/bsp_comapi.c
+++ b/board/p2041/src/bsp_comapi.c
@@ -7,6 +7,9 @@
 #include <net/if.h>
 #include <net/route.h>
 #include <sys/socket.h>
+#include <sys/resource.h>
+#include <sched.h>
+
 #define bcopy(src, dst, size)\
     memcpy(dst, src, size)
 void Thread_Entry1(int i);
@@ -21,6 +24,13 @@ static T_ThreadConfigInfo gaUserThreadConfigTable[] =
  //   { TRUE, &Thread_Entry4 ,99,BSP_COREID_3}
 };
 
+extern int sched_setaffinity (__pid_t __pid, size_t __cpusetsize,
+			      __const cpu_set_t *__cpuset) __THROW;
+extern void *mmap64 (void *__addr, size_t __len, int __prot,
+		     int __flags, int __fd, __off64_t __offset) __THROW;
+
+
+
 /********************************************************************************
 * 函数名称: netif_get_ip							
 * 功    能:                                     
@@ -70,14 +80,14 @@ int netif_get_ip(const char *ifname, unsigned char *ipaddr)
 ULONG BspGetCtrlNetIp(UCHAR * pucAddr) 
 {
     UCHAR ip[4] = {0};
-    if (BSP_OK != netif_get_ip((char*)CTRL_NET_ETH,ip))
+    if (BSP_OK != netif_get_ip((char*)CTRL_NET_ETH,(unsigned char *)ip))
     {
         printf("bspgetctrlnetip error!\n");
         return BSP_ERROR;
     }
     else
     {
-        sprintf(pucAddr, "%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
+        sprintf((char *)pucAddr, "%d.%d.%d.%d",(int)ip[0],(int)ip[1],(int)ip[2],(int)ip[3]);
 	 //printf("pucAddr->%s\n",pucAddr);
         return BSP_OK;
     }
@@ -237,7 +247,7 @@ void bsp_load_product_app(void)
 {
     WORD32  dwProcessId = 0;
     dwProcessId = bsp_create_process(APP_PROCESS_NAME,0);/*启动应用进程 */
-    printf("dwProcessId=%d\n",dwProcessId);
+    printf("dwProcessId=%d\n",(int)dwProcessId);
     if ((WORD32)(-1) == dwProcessId)
     {
 	      printf("main: detect Single Process !\n");
@@ -309,7 +319,7 @@ void Thread_Entry1(int i)
         return ; 
     cpumask = (1 << i);
     printf("now init webserver!\n");
-    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
+    if (sched_setaffinity(0, sizeof(cpumask), (cpu_set_t *)&cpumask) < 0)
     {
         printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
         return ;
@@ -327,7 +337,7 @@ void Thread_Entry2(int i)
         return ; 
     cpumask = (1 << i);
     printf("loding Thread_Entry2!!\n");
-    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
+    if (sched_setaffinity(0, sizeof(cpumask), (cpu_set_t *)&cpumask) < 0)
     {
         printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
         return ;
@@ -345,7 +355,7 @@ void Thread_Entry3(int i)
         return ; 
     cpumask = (1 << i);
     printf("loding Thread_Entry3!!\n");
-    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
+    if (sched_setaffinity(0, sizeof(cpumask), (cpu_set_t *)&cpumask) < 0)
     {
         printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
         return ;
@@ -363,7 +373,7 @@ void Thread_Entry4(int i)
         return ; 
     cpumask = (1 << i);
     printf("loding Thread_Entry4!!\n");
-    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
+    if (sched_setaffinity(0, sizeof(cpumask), (cpu_set_t *)&cpumask) < 0)
     {
         printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
         return ;
@@ -384,7 +394,7 @@ int bsp_attach_core(T_ThreadConfigInfo *pThreadTypeTable)
     pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
     param.sched_priority = (pThreadTypeTable)->dwPrio; 
     pthread_attr_setschedparam(&attr, &param);
-    result = pthread_create(&rx_thread, &attr, (void *)((pThreadTypeTable)->pEntry),(pThreadTypeTable)->dwCoreId);
+    result = pthread_create(&rx_thread, &attr, (void *)((pThreadTypeTable)->pEntry),(void *)(pThreadTypeTable)->dwCoreId);
     if (result != 0)
     {
         printf("pthread_create rx_thread failed\n");
diff --git a/board/p2041/src/fsl_p2041.c b/board/p2041/src/fsl_p2041.c
index 5be67d5..5c23519 100644
--- a/board/p2041/src/fsl_p2041.c
+++ b/board/p2041/src/fsl_p2041.c
@@ -53,6 +53,22 @@ extern UINT32 bsp_sharemem_init();
 extern void bsp_sys_msdelay(ULONG dwTimeOut);
 extern void bsp_sys_usdelay(ULONG dwTimeOut);
 extern void bsp_ushell_init(UCHAR * ucpsymbname, UCHAR * ucpboardname);
+extern void *BspShmemVirtMalloc(unsigned long align, unsigned long size, const char *name, unsigned long index,int *pdwphyaddr);
+extern unsigned long BspAlignSize(unsigned long align, unsigned long size);
+extern s32 bsp_i2c_init(void);
+extern void bsp_spi_init(void);
+extern void bsp_ethsw_spi_init(void);
+extern void BspAttachCoreTest(void );
+extern Queue *InitQueue(void );
+extern void BspRecvInit(void );
+extern void d4 (ULONG uladdr,ULONG len);
+extern char * readline (char * cmd);
+extern void *mmap64 (void *__addr, size_t __len, int __prot,
+		     int __flags, int __fd, __off64_t __offset) __THROW;
+
+#ifndef CPU_FSL_SYS_BIT32_WIDTH
+#define __USE_LARGEFILE64
+#endif
 /*************************** 局部函数原型声明 **************************/
 
 /************************************ 函数实现 ************************* ****/
@@ -71,7 +87,6 @@ extern void bsp_ushell_init(UCHAR * ucpsymbname, UCHAR * ucpboardname);
 
 UINT32  BspMallocAddr(int len)
 {
-    unsigned long tmp;
     unsigned long  dwphybase = 0;
     if (len<0)
 		return 0;
@@ -96,7 +111,6 @@ unsigned long g_sharephyaddr;
 s32 bsp_p2041_init(void)
 {
     u32 fd;
-	 unsigned char auMacAddr[6];
     fd = open("/dev/mem",O_RDWR|O_SYNC);
     if (BSP_ERROR == fd)
     {
@@ -106,7 +120,7 @@ s32 bsp_p2041_init(void)
     #ifdef CPU_FSL_SYS_BIT32_WIDTH
         g_u8ccsbar = mmap((void *)0,P2041_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,P2041_BASE);
     #else
-	    g_u8ccsbar = mmap64((void *)0,P2041_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,P2041_BASE);
+	    g_u8ccsbar = (u8 *)mmap64((void *)0,P2041_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,P2041_BASE);
     #endif
 	  
     if (BSP_ERROR == (s32)g_u8ccsbar)
@@ -119,7 +133,7 @@ s32 bsp_p2041_init(void)
     #ifdef CPU_FSL_SYS_BIT32_WIDTH
         g_u8fpgabase = mmap((void *)0,FPGA_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,FPGA_BASE);
     #else
-        g_u8fpgabase = mmap64((void *)0,FPGA_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,FPGA_BASE);
+        g_u8fpgabase = (u8 *)mmap64((void *)0,FPGA_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,FPGA_BASE);
     #endif
 		
     if (BSP_ERROR == (s32)g_u8fpgabase)
@@ -131,7 +145,7 @@ s32 bsp_p2041_init(void)
     #ifdef CPU_FSL_SYS_BIT32_WIDTH
         g_u8epldbase = mmap((void *)0,EPLD_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,EPLD_BASE);
     #else
-        g_u8epldbase = mmap64((void *)0,EPLD_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,EPLD_BASE);
+        g_u8epldbase = (u8 *)mmap64((void *)0,EPLD_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,EPLD_BASE);
     #endif
 	
     if (BSP_ERROR == (s32)g_u8epldbase)
@@ -200,15 +214,14 @@ void bsp_test_demo(void)
 {
     u32 fd;
 	u8 *g_qmanbar = 0;
-	u8 *g_bmanbar = 0;
     fd = open("/dev/usdpaa",O_RDWR|O_SYNC);
     if (BSP_ERROR == fd)
     {
 	    bsp_dbg("can not open mem!\n");
-		return BSP_ERROR;
+		return;
     }
-	g_qmanbar = mmap64((void *)0,0x100,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0xff4200000);
-    d4(g_qmanbar,0x100);
+	g_qmanbar = (u8 *)mmap64((void *)0,0x100,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0xff4200000);
+    d4((ULONG)g_qmanbar,0x100);
 }
 
 void bsp_board_test(void)
@@ -228,7 +241,7 @@ void bsp_board_test(void)
         printf("100: 退出\n");
         printf("\n请选择:--> ");
         ulItemNum  = atoi((char *)readline(""));
-        printf("Select is : %d\n",ulItemNum);
+        printf("Select is : %d\n",(int)ulItemNum);
         switch (ulItemNum)
         {
         /* FPGA下载测试 */
@@ -261,7 +274,7 @@ void bsp_board_test(void)
 int main(int argc, char *argv[])
 {
 	  bsp_board_test();
-	  bsp_ushell_init("bsp_app","p2041");
+	  bsp_ushell_init((UCHAR *)"bsp_app",(UCHAR *)"p2041");
     return BSP_OK;
 }
 #endif
diff --git a/com_inc/bsp_shmem_ext.h b/com_inc/bsp_shmem_ext.h
index 7623886..3c5fc33 100644
--- a/com_inc/bsp_shmem_ext.h
+++ b/com_inc/bsp_shmem_ext.h
@@ -13,7 +13,7 @@
 Export s32 bsp_shmem_init(void);
 typedef struct tagtT_ShmemRegInfo
 {
-	unsigned char  *pName;
+	char  *pName;
 	WORD32         index;
     WORD32         dwPhyAddr;
 	WORD32         dwVirAddr;
@@ -27,26 +27,6 @@ typedef struct tagtT_ShmemRegInfo
 #define BMPOOLSHMEMSIZE    (13<<20) 
 #define P2PSHMEMSIZE       (40<<20)
 
-static T_ShmemRegInfo gaUserShmemConfigTable[] =
-{
-	{"dpaheap",  0,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
-	{"dpaheap",  1,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
-	{"dpaheap",  2,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
-	{"dpaheap",  3,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
-	{"bmpool",   0,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
-	{"bmpool",   1,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
-	{"bmpool",   2,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
-	{"bmpool",   3,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
-	{"p2p",      0,   0,   0,   P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
-	{"p2p",      1,   0,   0,   P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
-	{"p2p", 	 2,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
-	{"p2p", 	 3,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
-	{"p2p", 	 4,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
-	{"p2p", 	 5,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
-	{"p2p", 	 6,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
-	{"p2p", 	 7,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
-};
-
 
 #endif
 /******************************* 头文件结束 ********************************/
diff --git a/modules/corecomm/src/bsp_core.c b/modules/corecomm/src/bsp_core.c
index bcb51c9..0709e3c 100644
--- a/modules/corecomm/src/bsp_core.c
+++ b/modules/corecomm/src/bsp_core.c
@@ -171,8 +171,6 @@ extern ULONG BspGetMpicTBCurCnt(UINT32 udTimerIndex);
 UINT32  requestIrq(UINT32 vector, ia_irq_func_t isr_func,void *data)
 {
 	ia_ioctl_t devio;
-	UINT32 ret;
-	ret = OK;
 	devio.dev_id = 0;
 	devio.d0 = (unsigned int)vector;
 	devio.d1 = ia_mult_int_mem_addr;
@@ -662,10 +660,6 @@ int BspMpicGetDestRegAdrs(ULONG ulVector)
 ******************************************************************************/
 int BspMpicGetLevlRegAdrs(ULONG ulVector)
 {
-    ULONG ulCurCpuId = 0;
-	
-    ulCurCpuId = g_ulCurCpuId;
-	
     if ((ulVector < MPIC_VEC_EXT_IRQ0) || (ulVector > MPIC_VEC_SPUR_IRQ))
 		return (-2);
 	
@@ -1074,12 +1068,10 @@ ULONG BspErrorCheckInit(void)
     ULONG ulVector = 0;
     ULONG ulPolarity = 0;
 	  ULONG ulSense = 0;
-	  ULONG ulPriority = 0;
     /*BspMpicCurTaskPrioSet(MPIC_PRIORITY_MAX);*/
 	  ulVector = MPIC_VEC_IN_IRQ0;
     /*ulSense = 0;*/
 	  ulPolarity = 1;
-	  ulPriority = MPIC_PRIORITY_DEFAULT;
 
     BspMpicIntDisable(MPIC_IN_INT0_VEC_REG);
     /* 中断源配置 */
@@ -1623,7 +1615,6 @@ ULONG BspINTEventIsr(ULONG ulPara)
 ******************************************************************************/
 void BspTestIpiIsr(ULONG ulVector)
 {
-    unsigned int uireg =0;
     switch(ulVector)
     {
         
diff --git a/modules/corecomm/src/timer.c b/modules/corecomm/src/timer.c
index cae991e..2484efd 100644
--- a/modules/corecomm/src/timer.c
+++ b/modules/corecomm/src/timer.c
@@ -15,13 +15,17 @@ typedef struct
 	
 }GLOBAL_TIMER_CALLBACK_FUNC;
 
-GLOBAL_TIMER_CALLBACK_FUNC pTimer_Fun[8]={NULL};
+GLOBAL_TIMER_CALLBACK_FUNC pTimer_Fun[8];
 
 ULONG g_ulCallBackParam[8] = {0}; 
 extern ULONG g_ulCurCpuId;
 
 ULONG g_ulTestMpicTimer = 0;
 
+extern UINT32 BspGetCurCpu(void );
+extern VOID BspIntcEnable(const ULONG ulIntNo);
+extern char * readline (char * cmd);
+
 
 /************************************************************/
 /*   内部函数声明                                           */
@@ -86,7 +90,7 @@ UINT32 BspTimerInit(UINT32 udTimerIndex,T_TimerInitParam *ptTmrInitParam)
 	    /*不为空则安装回调函数*/
 		#if 1
 
-		pTimer_Fun[udTimerIndex].pgTimerCallBack = ptTmrInitParam->pTimerCB;
+		pTimer_Fun[udTimerIndex].pgTimerCallBack = (void (*)())ptTmrInitParam->pTimerCB;
 	    g_ulCallBackParam[udTimerIndex] = ptTmrInitParam->udIntParam;
   
 		#else
@@ -386,7 +390,7 @@ UINT32 BspTimerCBInstall(UINT32 udTimerIndex, T_TimerCBParam *ptCBParam)
 		return BSP_E_INPUTPARA_ERROR;
 	}
 
-	pTimer_Fun[udTimerIndex].pgTimerCallBack = ptCBParam->pfTimerCB;
+	pTimer_Fun[udTimerIndex].pgTimerCallBack = (void (*)())ptCBParam->pfTimerCB;
 	g_ulCallBackParam[udTimerIndex] = ptCBParam->udIntParam;
 
 	//printf("pTimer_Fun 's addr = 0x%x ptCBParam->pfTimerCB =0x%x\n",pTimer_Fun,ptCBParam->pfTimerCB);
@@ -534,10 +538,10 @@ int BspMpicTimerConfig(ULONG ulTimerIndex,T_MPIC_TIMER_CONFIG * ptTimerCfg )
  
 ULONG BspMpicTimeBaseInit(void)
 {
-    T_MPIC_TIMER_CONFIG tMpicTimerConfig;
 	ULONG ulFrq = 0;
 
 #ifdef MPIC_TIME_BASE_CASCAD
+    T_MPIC_TIMER_CONFIG tMpicTimerConfig;
     /* 设置TimerB 第0 、1 号定时器级联为一个定时器 
        可以作为一个全局的时间戳给各个核来用*/
 	tMpicTimerConfig.ucCascadeTime = 1;
@@ -551,12 +555,6 @@ ULONG BspMpicTimeBaseInit(void)
 	/*BspTimerSetBaseCount(4,0x7fffffff);*/
 	
 #else
-    /* 设置TimerB 第0 号定时器为一个计数器 
-       可以作为一个全局的时间戳给各个核来用*/
-    tMpicTimerConfig.ucCascadeTime = 0;/* 不采用级联方式定时 */
-    tMpicTimerConfig.ucClockRatio  = 0;/* 采用默认分频 */
-	tMpicTimerConfig.ucRealTimeMod = 0;/* 内部时钟 */
-	tMpicTimerConfig.ucRollOver    = 0;/* 非级联模式时不会rollover  */
     /* 设置定时器基值 */
 	BspTimerSetBaseCount(TIME_BASE_TIMER_INDEX,0x7fffffff);
 #endif
@@ -635,11 +633,11 @@ void BspShowCallBack(void)
     {
         if(pTimer_Fun[ulCnt].pgTimerCallBack==NULL)
         {
-            printf("Timer Call Back Func[%d] is NULL\n",ulCnt);
+            printf("Timer Call Back Func[%d] is NULL\n",(int)ulCnt);
         }
 		else
 		{
-		    printf("Timer Call Back Func[%d] is Install\n",ulCnt);
+		    printf("Timer Call Back Func[%d] is Install\n",(int)ulCnt);
 		}
     }
 }
@@ -649,10 +647,9 @@ void BspMpicTimerTestInit(ULONG ulTimerIndex,ULONG ulTimeVal)
 {
     T_TimerInitParam tTimeInitPara;
 	/*T_TimerCBParam tTimerCBPara;*/
-	ULONG ulTemp = 0;
 	ULONG ulCpu = 0;
 	
-	printf("定时器号 : [%d] Time:[%d]ms\n",ulTimerIndex,ulTimeVal);
+	printf("定时器号 : [%lx] Time:[%lx]ms\n",ulTimerIndex,ulTimeVal);
 	
 	tTimeInitPara.udPrd = ulTimeVal;
 	tTimeInitPara.udClkFreq = g_ulTimerClkFreq/1000;/*ms级*/
@@ -704,9 +701,8 @@ void BspStopIntTest(ULONG ulTimerIndex)
 	
 }
 
-int BspMpicTimerTestStart(ULONG ulTimerIndex)
+void BspMpicTimerTestStart(ULONG ulTimerIndex)
 {
-    ULONG ulTemp = 0;
 	ULONG ulItemNum = 0;
 	if(ulTimerIndex < 4)
     {
@@ -721,17 +717,16 @@ int BspMpicTimerTestStart(ULONG ulTimerIndex)
 
 	printf("Please Input number 100 ,than 'Enter' to Exit!!!\n");
 
-	ulItemNum  = atoi(readline(""));
-    printf("Select is : %d\n",ulItemNum);
+	ulItemNum  = (ULONG)atoi(readline(""));
+    printf("Select is : %d\n",(int)ulItemNum);
 	while(100 != ulItemNum)
 	{
-	    ulItemNum  = atoi(readline(""));
-		printf("Select is : %d\n",ulItemNum);
+	    ulItemNum  = (ULONG)atoi(readline(""));
+		printf("Select is : %d\n",(int)ulItemNum);
 		printf("Please Input number 100 ,than 'Enter' to Exit!!!\n");
 		
 	}
 	BspStopIntTest(ulTimerIndex);
-
 }
 
 
diff --git a/modules/csi/inc/csi.h b/modules/csi/inc/csi.h
deleted file mode 100644
index 99884d2..0000000
--- a/modules/csi/inc/csi.h
+++ /dev/null
@@ -1,16 +0,0 @@
-#ifndef _CSI_H
-#define _CSI_H
-
-#include <bits/pthreadtypes.h>
-#include <sys/prctl.h>
-#include <unistd.h>
-#include <semaphore.h>
-#include <sys/time.h>
-
-#define   __NR_Linux			(6000)
-#define   __NR_gettid			(__NR_Linux + 178)
-#define   PPC_FUNC_PREFACE      (0x94210000)         /*PPC函数序言*/
-#define   PPC_FUNC_EPILOGUE     (0x4e800020)         /*PPC函数尾声*/
-#define   MFSPR_R0_LR           (0x7c0802a6)         /*mfspr r0, LR 的机器码*/
-
-#endif
diff --git a/modules/csi/makefile b/modules/csi/makefile
deleted file mode 100644
index 86c56e6..0000000
--- a/modules/csi/makefile
+++ /dev/null
@@ -1,25 +0,0 @@
-#*******************************************************************************
-#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
-#********************************************************************************
-#* 源文件名:   makefile for root dir
-#* 功能:       根据编译选项编译不同的目标程序  
-#* 版本:       v0.1                                                          
-#* 编制日期:   201307096                           
-#* 作者:       syl                                      
-#*******************************************************************************/ 
-DIR_TEMP = $(shell ls -d */)
-SUBDIRS_TEMP = $(subst /, ,$(DIR_TEMP))
-PARTERNAL = %inc
-SUBDIRS = $(filter-out $(PARTERNAL),$(SUBDIRS_TEMP))
-
-all: 
-	for dir in $(SUBDIRS); \
-	do $(MAKE) -C $$dir all || exit 1; \
-	done
-	
-clean:
-	@for dir in $(SUBDIRS); do make -C $$dir clean|| exit 1; done
-
-	$(RM) $(__OBJS) $(LIB) *.bak *~
-
-.PHONY: all clean
diff --git a/modules/csi/src/csi.c b/modules/csi/src/csi.c
deleted file mode 100644
index ffd5c74..0000000
--- a/modules/csi/src/csi.c
+++ /dev/null
@@ -1,21 +0,0 @@
-#include "csi.h"
-
-
-unsigned long bsp_get_thread_id(void)
-{
-    unsigned long  dwSysTaskId;
-    dwSysTaskId = pthread_self();
-    return dwSysTaskId;
-}
-
-pid_t gettid(VOID)
-{
-    return (pid_t)syscall(__NR_gettid);
-}
-
-void bsp_get_reg(void)
-{
-    unsigned long	  wThreadId;
-    wThreadId = bsp_get_thread_id();
-}
-
diff --git a/modules/csi/src/makefile b/modules/csi/src/makefile
deleted file mode 100644
index eaedef4..0000000
--- a/modules/csi/src/makefile
+++ /dev/null
@@ -1,35 +0,0 @@
-#*******************************************************************************
-#*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
-#********************************************************************************
-#* 源文件名:   makefile for root dir
-#* 功能:       根据编译选项编译不同的目标程序  
-#* 版本:       v0.1                                                          
-#* 编制日期:   201307096                           
-#* 作者:       syl                                      
-#*******************************************************************************/ 
-ifeq ($(PRODUCT), BSP)
-OUTPUTPATH = $(BSPOUTPUTPATH)
-endif
-
-ifeq ($(PRODUCT), PLATFORM)
-OUTPUTPATH =  $(PLATFORMOUTPUTPATH)
-endif
-
-ifeq ($(PRODUCT), MCWILL)
-OUTPUTPATH =  $(MCWILLOUTPUTPATH)
-endif
-
-ifeq ($(PRODUCT), LTE)
-OUTPUTPATH =  $(LTEOUTPUTPATH)
-endif
-
-SRCC = $(shell ls *.c)
-OBJS := $(patsubst %.c, %.o,$(SRCC))
-
-all: $(OBJS)
-	$(shell cp $(OBJS) $(OUTPUTPATH))
-
-clean:
-	$(RM) *.o *.bak 
-	
-.PHONY: all clean
\ No newline at end of file
diff --git a/modules/dsp/inc/bsp_dsp.h b/modules/dsp/inc/bsp_dsp.h
index 50f5895..e68f3dc 100644
--- a/modules/dsp/inc/bsp_dsp.h
+++ b/modules/dsp/inc/bsp_dsp.h
@@ -50,7 +50,7 @@ const s8 dsp3_download_ip[] = "10.0.0.6";
 const s8 dsp4_download_ip[] = "10.0.0.7";
 
 s8  *pDownloadIp = NULL;
-s8  *pMPC_to_dsp_Ip = "10.0.0.1";
+char  *pMPC_to_dsp_Ip = "10.0.0.1";
 
 u8  g_u8Dsp1DlFlag = BBU_DSP_NOT_DOWNLOADED;         /* flag for dsp loaded or not */
 u8  g_u8Dsp2DlFlag = BBU_DSP_NOT_DOWNLOADED;         /* flag for dsp loaded or not */
diff --git a/modules/dsp/src/bsp_dsp.c b/modules/dsp/src/bsp_dsp.c
index 1f41c18..1de59a5 100644
--- a/modules/dsp/src/bsp_dsp.c
+++ b/modules/dsp/src/bsp_dsp.c
@@ -706,7 +706,7 @@ s32 bsp_dsp_download(u32 u32DspId, const s8* pFileTem)
 			{
 				s32NumBytes = 0;
 				s32socketlen = sizeof(struct sockaddr);
-				if (SOCKET_ERROR == (s32NumBytes = recvfrom(g_s32SocketFdBootp,s8Buf,(s32)BBU_DSP_MAX_BUF_LENGTH,0,(struct sockaddr*)&struOppositeAddr,(int *)&s32socketlen)))
+				if (SOCKET_ERROR == (s32NumBytes = recvfrom(g_s32SocketFdBootp,s8Buf,(s32)BBU_DSP_MAX_BUF_LENGTH,0,(struct sockaddr*)&struOppositeAddr,(socklen_t *)&s32socketlen)))
 				{
 					s32NumBytes = s32NumBytes;
 					printf("[fdd_dsp_download]:failed! faild to receive the bootp packet!\n");
diff --git a/modules/epld/src/bsp_epld.c b/modules/epld/src/bsp_epld.c
index 6cf31d2..6bfccf2 100644
--- a/modules/epld/src/bsp_epld.c
+++ b/modules/epld/src/bsp_epld.c
@@ -3248,6 +3248,8 @@ XO2FeatureRow_t cpld_FeatureRow =
 
 extern void bsp_sys_msdelay(ul32 dwTimeOut);// ---延迟单位为毫秒
 extern void bsp_sys_usdelay(ul32 dwTimeOut); //---延迟单位为微妙
+extern s32 spi_cpld_read(u8 *cmd,u8 cmd_len,u8 *u8cpld_data,u32 len);
+extern s32 spi_cpld_write(u8 *u8cpld_data,u32 len);
 
 /***************************函数实现 *****************************/
 
@@ -3305,7 +3307,7 @@ u8 read_busy(void)
 * 参数名称		   类型					输入/输出 		描述	
 * 说   明: 
 *********************************************************************************/
-s32 read_status(void)
+void read_status(void)
 {
 	u8 write_status[4] ={0x3C,0,0,0};
 	u8 read_status[4];
@@ -3346,9 +3348,9 @@ s32 cpld_delay(void)
 * 参数名称		   类型					输入/输出 		描述	
 * 说   明: 
 *********************************************************************************/
-s32 program_cpld(u8 *data,u8 len)
+void program_cpld(u8 *data,u8 len)
 {
-    char data1[20]={0};
+    u8 data1[20]={0};
 	data1[0] = 0x70;
 	data1[1] = 0;
 	data1[2] = 0;
@@ -3366,9 +3368,9 @@ s32 program_cpld(u8 *data,u8 len)
 * 参数名称		   类型					输入/输出 		描述	
 * 说   明: 
 *********************************************************************************/
-s32 write_feature(u8 *data,u8 len)
+void write_feature(u8 *data,u8 len)
 {
-    char data1[12]={0};
+    u8 data1[12]={0};
 	data1[0] = 0xE4;
 	data1[1] = 0;
 	data1[2] = 0;
@@ -3386,9 +3388,9 @@ s32 write_feature(u8 *data,u8 len)
 * 参数名称		   类型					输入/输出 		描述	
 * 说   明: 
 *********************************************************************************/
-s32 write_feabits(u8 *data,u8 len)
+void write_feabits(u8 *data,u8 len)
 {
-    char data1[6]={0};
+    u8 data1[6]={0};
 	data1[0] = 0xF8;
 	data1[1] = 0;
 	data1[2] = 0;
@@ -3557,7 +3559,7 @@ void read_dev_id(void)
 	u8 data[4];
 	spi_cpld_read(cmd,4,data,4);
 	if(cpld_delay())
-		return BSP_ERROR;
+		return;
     printf("cpld device id =%x %x %x %x\n",data[0],data[1],data[2],data[3]);
 }
 
@@ -3576,7 +3578,7 @@ void read_trace_id(void)
 	u8 data[8];
 	spi_cpld_read(cmd,4,data,8);
 	if(cpld_delay())
-		return BSP_ERROR;
+		return;
     printf("data=%x %x %x %x %x %x %x %x\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
 }
 
@@ -3595,7 +3597,7 @@ void read_usr_id(void)
 	u8 data[4];
 	spi_cpld_read(cmd,4,data,4);
 	if(cpld_delay())
-		return BSP_ERROR;
+		return;
     printf("data=%x %x %x %x\n",data[0],data[1],data[2],data[3]);
 }
 
@@ -3612,16 +3614,15 @@ const s8 cpld_bin_name[]= "/dev/shm/cpld.jed";
 u8 verify = 1;
 s32 cpld_download(void)
 {  
-    FILE *file_cpld;
+    //FILE *file_cpld;
 	s32 counter,remainder;	
-	u8 u8ufm[16];
-	u8 u8data[16];
+	//u8 u8ufm[16];
+	//u8 u8data[16];
 	u16 flag = 0;
 	u32 count = 0;
 	u32 count1 = 0;
-	struct stat *cpld_file_stat;
+	//struct stat *cpld_file_stat;
     u32 cpld_size;
-	s32 i;
 	s32 cf_size = 0;
 
     //stat(cpld_bin_name,cpld_file_stat);
@@ -3701,7 +3702,7 @@ s32 cpld_download(void)
 			printf("fread  fail222!! \n");
 			return BSP_ERROR;
 		}*/
-		program_cpld(cpld_CfgData[cf_size],remainder);
+		program_cpld(&cpld_CfgData[cf_size],(u8)remainder);
 		if(cpld_delay())
 			return BSP_ERROR;
 	}
@@ -3730,7 +3731,6 @@ s32 cpld_download(void)
 *********************************************************************************/
 s32 bsp_cpld_boot(void)
 {
-	FILE *file_cpld;
 	s32 ret;
 	s32 i = 0;
 	s32 count = 0;
@@ -3758,7 +3758,7 @@ s32 bsp_cpld_boot(void)
 	while(i < 1)
     {
 	    /* config cpld */
-	    cpld_download();	
+	    ret = cpld_download();	
 		if(ret < 0)
 		{
 			i++;
diff --git a/modules/ethsw/src/bsp_ethsw_app.c b/modules/ethsw/src/bsp_ethsw_app.c
index 7ae234e..62b658d 100644
--- a/modules/ethsw/src/bsp_ethsw_app.c
+++ b/modules/ethsw/src/bsp_ethsw_app.c
@@ -286,7 +286,6 @@ s32 ethsw_get_port_mib(u8 u8PortId)
 {
 	u8   u8Buf[8];
 	u32 u32temp1;
-	u32 u32temp2;
 
 	if(u8PortId > MAX_USED_PORTS - 1)
 	{
diff --git a/modules/ethsw/src/bsp_ethsw_port.c b/modules/ethsw/src/bsp_ethsw_port.c
index 53509aa..d998485 100644
--- a/modules/ethsw/src/bsp_ethsw_port.c
+++ b/modules/ethsw/src/bsp_ethsw_port.c
@@ -287,7 +287,7 @@ void ethsw_set_mactest(void)
 
 }
 #if 1
-void set_mirror_test1(void)
+int set_mirror_test1(void)
 {
 
     u16 test;
@@ -306,7 +306,7 @@ void set_mirror_test1(void)
     RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, (u8 *)&test, TWO_BYTE_WIDTH));
 		printf("MIRROR_CAPTURE_CTRL");
 
-    return;
+    return 0;
 
 }
 
diff --git a/modules/ethsw/src/bsp_ethsw_spi.c b/modules/ethsw/src/bsp_ethsw_spi.c
index 7908b8e..bdb69d2 100644
--- a/modules/ethsw/src/bsp_ethsw_spi.c
+++ b/modules/ethsw/src/bsp_ethsw_spi.c
@@ -75,7 +75,7 @@ s32 bsp_ethsw_spi_setup(u32 u32cs,u32 u32max_hz,u32 u32mode)
 	u8 u8pm = 0;
 	u32  u32div16;
 	s32 i;
-	u32 u32temp;
+	//u32 u32temp;
 	if (u32cs >= ESPI_MAX_CS_NUM)
 		return BSP_ERROR;
 
@@ -144,7 +144,7 @@ s32 bsp_ethsw_spi_setup(u32 u32cs,u32 u32max_hz,u32 u32mode)
 	/* Character length in bits, between 0x3~0xf, i.e. 4bits~16bits */
 	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], spi_read_reg(&ethsw_spi_reg->csmode[u32cs])
 		| ESPI_CSMODE_LEN(0x7));
-	u32temp = spi_read_reg(&ethsw_spi_reg->csmode[u32cs]);
+	//u32temp = spi_read_reg(&ethsw_spi_reg->csmode[u32cs]);
 	spi_write_reg(&ethsw_spi_reg->csmode[u32cs], 0x74171108);
 
 	return BSP_OK;
@@ -170,7 +170,7 @@ Private void spi_cs_activate(u32 u32cs,u32 u32tran_len)
 
 int ethsw_spi_xfer(const void *pdata_out, u32 u32data_len)
 {
-	unsigned int u32tmpdout, u32tmpdin, u32event;
+	unsigned int u32tmpdout, u32event;
 	const void *dout = NULL;
 	int len = 0;
 	int num_blks, num_chunks, max_tran_len, tran_len;
@@ -220,7 +220,6 @@ int ethsw_spi_xfer(const void *pdata_out, u32 u32data_len)
              ETHSW_DELAY(100);
 			u32event = spi_read_reg(&ethsw_spi_reg->event);
 			if (u32event & ESPI_EV_RNE) {
-				u32tmpdin = spi_read_reg(&ethsw_spi_reg->rx);
 				spi_write_reg(&ethsw_spi_reg->event, spi_read_reg(&ethsw_spi_reg->event)| ESPI_EV_RNE);
 			}
 		}
@@ -232,13 +231,9 @@ int ethsw_spi_xfer(const void *pdata_out, u32 u32data_len)
 
 static s32 ethsw_spi_poll_rw_done(void)
 {
-    u8   u8Write[2] = {0};
     u8 u8read[4];
     u32  u32i;
-    unsigned char *buffer = NULL;
-    const void *dout = NULL;
     unsigned int u32tmpdout, u32tmpdin, u32event;
-    unsigned char *ch;
     /* Timeout after 50 tries without MDIO_START low */
 	for (u32i = 0; u32i < SPI_DEFAULT_TIMEOUT; u32i++)
 	{
@@ -392,7 +387,6 @@ s32 ethsw_read_reg(u8 u8RegPage, u8 u8RegAddr, u8 *pu8Buf, u8 u8Len)
 	unsigned char *ch;
 	unsigned int num_blks = u8Len / 4 + (u8Len % 4 ? 1 : 0);
     unsigned int num_bytes = u8Len;
-    s32 s32Rv;
 
 	dout = pu8Buf;
     bsp_ethsw_spi_init();
diff --git a/modules/fpga/src/bsp_fpga.c b/modules/fpga/src/bsp_fpga.c
index 31539a4..83d6db9 100644
--- a/modules/fpga/src/bsp_fpga.c
+++ b/modules/fpga/src/bsp_fpga.c
@@ -99,7 +99,7 @@ u16 bsp_fpga_read_reg(u16 u16Reg_offset)
 *******************************************************************************/
 void bsp_fpga_write_reg(u16 u16Reg_offset,u16 u16Dat)
 {
-	bsp_write_reg(g_u8fpgabase+u16Reg_offset,u16Dat);
+	bsp_write_reg((u16 *)(g_u8fpgabase+u16Reg_offset),u16Dat);
 	//printf("the W_addr is 0x%x\n",(u32)g_u8fpgabase+u16Reg_offset);
 }
 
@@ -135,7 +135,7 @@ u16 bsp_cpld_read_reg(u16 u16Reg_offset)
 *******************************************************************************/
 void bsp_cpld_write_reg(u16 u16Reg_offset,u16 u16Dat)
 {
-	bsp_write_reg(g_u8epldbase+u16Reg_offset,u16Dat);
+	bsp_write_reg((u16 *)(g_u8epldbase+u16Reg_offset),u16Dat);
 	//printf("the W_addr is 0x%x\n",(u32)g_u8epldbase+u16Reg_offset);
 }
 
@@ -254,11 +254,10 @@ s32 fpga_download()
 	u32 delayCount = 0;	
 	u8 u8data;
 	u16 u16data;
-	u32 count;
 	s32 i = 0;
 	s32 j = 0;
 
-	file_fpga = fopen(fpga_bin_name,"rb");
+	file_fpga = fopen((char *)fpga_bin_name,"rb");
 	if (file_fpga == NULL)
 	{
 		printf("%s open fail!! \n",fpga_bin_name);   
@@ -291,7 +290,7 @@ s32 fpga_download()
 	while ( (!feof(file_fpga)) )  
 	{
 		//读取1个字节
-		count = fread(&u8data, 1, 1, file_fpga); 
+		fread(&u8data, 1, 1, file_fpga); 
 		for(j = 0;j < 8;j++)
 		{
 			if((u8data >> j)&0x1)
@@ -367,7 +366,6 @@ s32 bsp_boot_fpga(void)
 /************   测试 cpld fpga 寄存器读写,nandflash读写*****************/
 void cpld_wr_test(void)
 {
-    u16 *p_fpga_data;
 	u16  cpld_data;
     bsp_cpld_write_reg(0x58,0x9876);
 	
@@ -379,7 +377,6 @@ void cpld_wr_test(void)
 
 void fpga_wr_test(void)
 {
-    u16 *p_fpga_data;
 	u16  fpga_data;
     bsp_fpga_write_reg(0x58,0x9876);
 	
@@ -431,13 +428,14 @@ int writeNvramToFlash(char *srcData, UINT32 len)
 	}
 	fclose(fdhead);	   
     
+	return 0;
 }
 
 void nand_write_read_test()
 {
-	FILE *fp;
+	//FILE *fp;
 	int i;
-    char name[]= "/mnt/nand_test";
+    //char name[]= "/mnt/nand_test";
 	char s[100]={0};
 
 	for (i=0;i<100;i++)
diff --git a/modules/gps/src/bsp_gps.c b/modules/gps/src/bsp_gps.c
index 8e93552..35320d4 100644
--- a/modules/gps/src/bsp_gps.c
+++ b/modules/gps/src/bsp_gps.c
@@ -355,7 +355,7 @@ void t_gps_sched_proc (void)
 {
 	u8 u8i;
 	u8 u8norcvtodcounter[GPS_TOD_CHANNEL_NUM];
-	u32 u32SysTickRate;// = (u32)sysClkRateGet();
+//	u32 u32SysTickRate;// = (u32)sysClkRateGet();
 
 	for(u8i=0;u8i<GPS_TOD_CHANNEL_NUM;u8i++)
 	{
@@ -1082,7 +1082,7 @@ SYS_STATUS  t_gps_tod_sec_exe(void)
 {
 	char  s8AlarmStr[50] = {0};
 	u8 u8DdToCcuAutoRptMsgTemp[DD_AUTO_RPT_MSG_LEN];
-	u32 u32SysTickRate;// = (u32)sysClkRateGet();
+	u32 u32SysTickRate = 100000;// = (u32)sysClkRateGet();
 
 	g_u64GpsCurrentTick = 0;
 	g_u64GpsLastTick = 0;
@@ -1216,8 +1216,8 @@ void gps_isr(int signo)
 	}
 	
 	sem_post(&g_sem_tod_sec_exe);
-	sem_post(&g_semb_getphas_sendac);
-	sem_post(&g_semb_pll_main_proc);
+	sem_post((sem_t *)&g_semb_getphas_sendac);
+	sem_post((sem_t *)&g_semb_pll_main_proc);
 	sem_post(&g_sem_sched_regulate);
 
 	if (GPS_PRINT_LEVEL_ALL < g_u32gpsprint_level)
@@ -1330,7 +1330,7 @@ u32  gps_round(float data)
 *******************************************************************************/
 SYS_STATUS ubx_init(void)
 {
-    u32 u32SysTickRate;/// = (u32)sysClkRateGet();
+    //u32 u32SysTickRate;/// = (u32)sysClkRateGet();
 
     (void)ubx_select_nav_sol(1);/*使能nav_sol输出*/
   //  (void)taskDelay((s32)u32SysTickRate);
@@ -2905,7 +2905,7 @@ SYS_STATUS ubx_tod_parse_entry(u32 u32uartindex, const u8 *pu8FrameHead, u32 u32
 {
 	float floatLatitude;
 	float floatLongitude;
-	s16 s16Altitude;
+	//s16 s16Altitude;
 
 	MSG_UBX_NAV_POSLLH     struNAV_POSLLHMsg;
 	MSG_UBX_NAV_SOL        struNAV_SOLMsg;
@@ -3039,7 +3039,7 @@ SYS_STATUS ubx_tod_parse_entry(u32 u32uartindex, const u8 *pu8FrameHead, u32 u32
 
 						floatLongitude = (float)g_struUBXInfo.Longitude/10000000;
 						floatLatitude = (float)g_struUBXInfo.Latitude/10000000;
-						s16Altitude = g_struGpsInfo.s16GpsAltitude;
+						//s16Altitude = g_struGpsInfo.s16GpsAltitude;
 
 						g_struUBXSingleMode.fLongitude = floatLongitude;
 						g_struUBXSingleMode.fLatitude = floatLatitude;
@@ -4231,7 +4231,7 @@ SYS_STATUS gps_ppc_uart_seri_default_set(void)
 *******************************************************************************/
 SYS_STATUS gps_all_seriopt_init (void)
 {
-	u32 u32SysTickRate;// = (u32)sysClkRateGet();
+	//u32 u32SysTickRate;// = (u32)sysClkRateGet();
 
 	/*设置处理器串口设备*/
 	if (GPS_OK != gps_ppc_uart_seri_set(9600,1))
diff --git a/modules/i2c/src/bsp_i2c.c b/modules/i2c/src/bsp_i2c.c
index bc9ce1e..ebb776e 100644
--- a/modules/i2c/src/bsp_i2c.c
+++ b/modules/i2c/src/bsp_i2c.c
@@ -35,6 +35,7 @@
 /*************************** 局部函数原型声明 **************************/
 extern void bsp_sys_msdelay(ul32 dwTimeOut);// ---延迟单位为毫秒
 extern void bsp_sys_usdelay(ul32 dwTimeOut); //---延迟单位为微妙
+extern void bsp_gpio_i2c_init(void );
 void i2c_init(int speed,int port);
 void bsp_current_monitor_init(void);
 s32 bsp_fan_config_init(void);
@@ -139,7 +140,6 @@ int i2c_wait(int write,int port)
 static unsigned int set_i2c_bus_speed(const struct fsl_i2c *dev,unsigned int i2c_clk, unsigned int speed)
 {
 	unsigned short divider = min(i2c_clk / speed, (unsigned short) -1);
-	UCHAR dfsr, fdr ; /* Default if no FDR found */
 	unsigned int i;
 	for (i = 0; i < ARRAY_SIZE(fsl_i2c_speed_map); i++)
 		if (fsl_i2c_speed_map[i].divider >= divider) 
@@ -170,7 +170,6 @@ void i2c_init(int speed,int port)
 {
 	struct fsl_i2c *dev;
 	unsigned int temp;
-	volatile void *piiccr;
 	switch(port)
 	{
         case P2041_IIC0:
@@ -192,6 +191,8 @@ void i2c_init(int speed,int port)
 	writeb(0, &dev->cr);	/* stop I2C controller */
 	bsp_sys_usdelay(5);	/* let it shutdown in peace */	
 	temp = set_i2c_bus_speed(dev,300000000,speed);
+	if (temp < speed)
+		printf("Set speed %d\n", (int)temp);
 	//writeb(0x3F, &dev->dfsrr);
 	//writeb(slaveadd<<1 , &dev->adr);/* not write slave address */
 	writeb(0x0, &dev->sr);			/* clear status register */
@@ -487,7 +488,7 @@ static int i2c_wait4bus(int port)  //busy?
 {
 	volatile void *piicsr;
 	int i = 0;
-	unsigned char itmp=0;
+	//unsigned char itmp=0;
 	switch(port)
 	{
 		case P2041_IIC0:
@@ -626,7 +627,6 @@ s32 bsp_read_eeprom(u8 *u8pread_data,u32 u32len,u16 u16addr)
 s32 bsp_write_eeprom(u8 *u8pwrite_data,u32 u32len, u16 u16addr)
 { 
 	u32 u32eepromDevId = 0;
-	u8 eerom_data;
 	s32 i;
 	for(i = 0;i < u32len;i++)
     {	
@@ -1047,7 +1047,7 @@ s32 bsp_read_current(f32 *f32pcurrent)
 *********************************************************************************/
 s32 bsp_read_power(f32 *f32p_power)
 {
-    u16 u16power_reg;
+    //u16 u16power_reg;
 	f32 f32pcurrent;
 	f32 f32pvoltage;
    /* if(bsp_read_power_sense(3,&u16power_reg)== BSP_ERROR)
@@ -1088,6 +1088,8 @@ s32 bsp_fan_config_init(void)
 		printf("set fan speed error!\n");
 		return BSP_ERROR;
 	}
+
+    return BSP_OK;
 }
 
 /********************************************************************************
@@ -1274,7 +1276,7 @@ void power_test(void)
 	bsp_read_power(&power);
 }
 
-void fan_test(u8 fan_pwmval)//0x80 半速
+int fan_test(u8 fan_pwmval)//0x80 半速
 {
 	u16 fan_speed;
 	u8  fan_pwmval1;
@@ -1293,6 +1295,8 @@ void fan_test(u8 fan_pwmval)//0x80 
 	}
 	
 	printf("fan_pwmval1 = 0x%x\n",fan_pwmval1);
+
+	return BSP_OK;
 }
 
 #endif
diff --git a/modules/i2c/src/bsp_i2c_gpio.c b/modules/i2c/src/bsp_i2c_gpio.c
index 46870a1..8c1ab41 100644
--- a/modules/i2c/src/bsp_i2c_gpio.c
+++ b/modules/i2c/src/bsp_i2c_gpio.c
@@ -62,7 +62,7 @@ void bsp_gpio_i2c_init(void)
 {	
     g_gpiocr = (volatile u32 *)((u32)g_u8ccsbar+0x130000);
 	g_gpiodr = (volatile u32 *)((u32)g_u8ccsbar+0x130008);
-	printf("g_gpiocr = 0x%x\n",g_gpiocr);
+	printf("g_gpiocr = 0x%x\n",(int)g_gpiocr);
 	printf("bsp_gpio_i2c_init!\n");
 	*g_gpiocr |= GPIO_SET_OUTPUT21|GPIO_SET_OUTPUT22;  
 }
@@ -70,7 +70,6 @@ void bsp_gpio_i2c_init(void)
 
 Private void gpio_i2c_wait(void)
 {
-	int i;
 	bsp_sys_usdelay(5);
 }
 
@@ -417,13 +416,14 @@ s32 bsp_get_spf_reg(u8 u8sfp_cs,u8 u8address,u8 *u8read_spf_data,u32 u8byte_num)
 
 
 
-void read_sfp(void)
+int read_sfp(void)
 {
 	 if(BSP_OK != bsp_get_spf_reg(1,0,g_BBU_Fiber_INfo,128))
 	{
 		printf("bsp_get_spf_reg error!\n");
 		return BSP_ERROR;
 	}
+	 return BSP_OK;
 }
 
 
@@ -441,7 +441,7 @@ s32 fiber_set_info_limint(void)
 }
 
 
-void bsp_get_fiber_Info(unsigned char flag,u8 u8sfp_cs,fiber_info *fiber_data)
+int bsp_get_fiber_Info(unsigned char flag,u8 u8sfp_cs,fiber_info *fiber_data)
 {
     s32 s32i;
 	float  *Rx_PWR4,*Rx_PWR3,*Rx_PWR2,*Rx_PWR1,*Rx_PWR0; 
@@ -526,7 +526,7 @@ void bsp_get_fiber_Info(unsigned char flag,u8 u8sfp_cs,fiber_info *fiber_data)
 	Temper = (T_S*(*T_AD) + (*T_O))*0.004;
 	if(g_print_bbu_fiber_info)
 	{
-		printf("BBU Fiber optic module Temperature:%6.2f(??)\n",Temper);
+		printf("BBU Fiber optic module Temperature:%6.2f(C)\n",Temper);
 	}
   
 
@@ -545,7 +545,7 @@ void bsp_get_fiber_Info(unsigned char flag,u8 u8sfp_cs,fiber_info *fiber_data)
 		printf("BBU Fiber optic module Voltuage:%8.2f(uV)\n",Vol);
 	}
 
- 
+	return BSP_OK; 
 }
 
 
@@ -621,7 +621,6 @@ void test_spf1(u8 cs,u8 device_id,u8 addr)
 
 void test_spf_all(u8 cs,u8 device_id)
 {
-    u8 spf[256];
     int i;
 	
     //gpio_i2c_init();
@@ -671,7 +670,7 @@ void clk_test0(void)
 
 void clk_test_ms(int time)
 {
-   int i;
+   //int i;
    
    bsp_gpio_i2c_init();
 
diff --git a/modules/pll/src/bsp_pll.c b/modules/pll/src/bsp_pll.c
index 85b5b95..649e6a9 100644
--- a/modules/pll/src/bsp_pll.c
+++ b/modules/pll/src/bsp_pll.c
@@ -97,6 +97,7 @@ s32 t_pll_get_freq_phase(void);
 s32 t_pll_func(void);
 s32 t_pll_lmk3000(void);
 void PLL_EPLD(void);
+extern s32 spi_dac_write(u8 u8_mod,u16 u16da_data);
 
 /******************************* 函数实现 *************************************/
 /*******************************************************************************
@@ -741,7 +742,7 @@ s32 pll_phase_modulate(void)
 *******************************************************************************/
 void  pll_epld_clk_rst_oc(void)
 {
-	u32 u32CcuTick ;//= (u32)sysClkRateGet();
+	//u32 u32CcuTick ;//= (u32)sysClkRateGet();
 
 	*EPLD_RESET_CLOLK_REG = 0x80;
 	//    (void)taskDelay(((int)(u32CcuTick/8)));
@@ -762,7 +763,7 @@ void  pll_epld_clk_rst_oc(void)
 *******************************************************************************/
 void  pll_epld_clk_rst_reg(void)
 {
-	u32 u32CcuTick ;//= (u32)sysClkRateGet();
+	//u32 u32CcuTick ;//= (u32)sysClkRateGet();
 
 	*EPLD_RESET_CLOLK_REG = 0x01;
 	//  (void)taskDelay(((int)(u32CcuTick/8)));
@@ -994,7 +995,7 @@ s32 t_pll_func(void)
 
 	u32 u32GpsClkAvailableCount = 0;
 	u32 u32GpsClkUnAvailableCount = 0;
-	u32 u32CcuTick ;//= (u32)sysClkRateGet();
+	//u32 u32CcuTick ;//= (u32)sysClkRateGet();
 	u16 u16LockTimes = 0;/*判断参考时钟源可用计数*/
 	u16 u16unlockTimes = 0;/*判断参考时钟源不可用计数*/
 	/*u16 u16unlockTimesTriple = 5;*/ /*用于增加一次调整EPLD寄存器的计数*/
diff --git a/modules/pll/src/bsp_pll_phase.c b/modules/pll/src/bsp_pll_phase.c
index 570ac7f..21c401a 100644
--- a/modules/pll/src/bsp_pll_phase.c
+++ b/modules/pll/src/bsp_pll_phase.c
@@ -48,7 +48,7 @@ extern u16 g_u16LagData;
 *******************************************************************************/
 void pll_freq_count_rst(void)
 {
-	u32 u32CcuTick;// = (u32)sysClkRateGet();
+	//u32 u32CcuTick;// = (u32)sysClkRateGet();
 
 	*EPLD_RESET_CLOLK_REG = 0x40;
 	//  (void)taskDelay(((int)(u32CcuTick/8)));
diff --git a/modules/shmem/src/bsp_shmem.c b/modules/shmem/src/bsp_shmem.c
index e0f0c68..82abca7 100644
--- a/modules/shmem/src/bsp_shmem.c
+++ b/modules/shmem/src/bsp_shmem.c
@@ -36,6 +36,26 @@
 #define BMAN_CACHE_MEM_TLB                         59
 #define BMAN_INCACHE_MEM_TLB                         57
 
+static T_ShmemRegInfo gaUserShmemConfigTable[] =
+{
+	{"dpaheap",  0,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
+	{"dpaheap",  1,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
+	{"dpaheap",  2,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
+	{"dpaheap",  3,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
+	{"bmpool",   0,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
+	{"bmpool",   1,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
+	{"bmpool",   2,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
+	{"bmpool",   3,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
+	{"p2p",      0,   0,   0,   P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
+	{"p2p",      1,   0,   0,   P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
+	{"p2p", 	 2,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
+	{"p2p", 	 3,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
+	{"p2p", 	 4,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
+	{"p2p", 	 5,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
+	{"p2p", 	 6,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
+	{"p2p", 	 7,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  TRUE},
+};
+
 /******************************************************************************
 * 函数名: BspShmP2V
 * 功  能: 
@@ -528,8 +548,6 @@ UINT32 bsp_sharemem_init()
 
 void  BspShmemInfoShow(void)
 {
-    unsigned long phyaddr;
-    TGshmMallocParam tGshmMallocParam;
     int fd_usdpaa;
     int ret = 0;
     if(g_dwShmHeapInitDone == 0)
@@ -541,7 +559,7 @@ void  BspShmemInfoShow(void)
     if (fd_usdpaa< 0) 
     {
     printf("can't open /dev/usdpaa device");
-    return 0;
+    return;
     }
     ret = ioctl(fd_usdpaa, USDPAA_IOC_GSHM_SHOW, NULL);
     if (ret < 0)
@@ -553,11 +571,10 @@ void  BspShmemInfoShow(void)
     close(fd_usdpaa);
 }
 
-void BspShowEmacDataCnt(void)
+int BspShowEmacDataCnt(void)
 {
 	volatile int ret;
 	int fd_usdpaa;
-	int fd_mem;
 	int ibytecnt=0;
 	fd_usdpaa = open("/dev/usdpaa", O_RDWR);
 	if (fd_usdpaa< 0) 
@@ -572,7 +589,9 @@ void BspShowEmacDataCnt(void)
 		close(fd_usdpaa);
 		return ret;
 	}
-    printf("get kernel pack count->0x%lx\n",ibytecnt);
+    printf("get kernel pack count->0x%x\n",ibytecnt);
+
+    return BSP_OK;
 }
 
 void *bsp_shm_p2v(unsigned long dwphyaddr)
@@ -643,7 +662,7 @@ ULONG bsp_config_sys_shm_table(void)
 	{
 	    bsp_dbg("can not open mem!\n");
 		close(fd);
-	    return NULL;
+	    return BSP_ERROR;
 	}
 	for (dwThreadTypeIndex = 0; dwThreadTypeIndex < dwTableItemCounts; dwThreadTypeIndex++)
 	{
@@ -660,7 +679,7 @@ ULONG bsp_config_sys_shm_table(void)
 					        PROT_READ|PROT_WRITE,MAP_SHARED,fd,(off_t)(void *)dwphyaddr);
 				if (dwviraddr>0)
 				{
-				    (gaUserShmemConfigTable + dwThreadTypeIndex)->dwVirAddr = (unsigned long *)dwviraddr;
+				    gaUserShmemConfigTable[dwThreadTypeIndex].dwVirAddr = (WORD32)dwviraddr;
 				}
 			}
 			else
@@ -686,7 +705,7 @@ ULONG bsp_find_phyaddr_from_tbl(unsigned char *pname,unsigned int dwindex)
 					   (gaUserShmemConfigTable + dwThreadTypeIndex)->dwLen >0 )
 	    {
 
-			if((!(strcmp(pname, (gaUserShmemConfigTable + dwThreadTypeIndex)->pName))) && \
+			if((!(strcmp((char *)pname, (gaUserShmemConfigTable + dwThreadTypeIndex)->pName))) && \
 				 (dwindex == (gaUserShmemConfigTable + dwThreadTypeIndex)->index))
 			{
 			    dwphyaddr = (gaUserShmemConfigTable + dwThreadTypeIndex)->dwPhyAddr;
diff --git a/modules/shmem/src/shmem.c b/modules/shmem/src/shmem.c
index 0f178a3..4cb6e8c 100644
--- a/modules/shmem/src/shmem.c
+++ b/modules/shmem/src/shmem.c
@@ -96,7 +96,9 @@ void * fsl_shmem_memalign(size_t align, size_t size)
 * 2013/07/10    V1.0           
 ************************************************************************/
 int g_dwShMemSetup = 0;
+#undef FSL_SHMEM_PHYS
 #define FSL_SHMEM_PHYS	(u32)0x70000000 /* 1.75G */
+#undef FSL_SHMEM_SIZE
 #define FSL_SHMEM_SIZE	(u32)0x10000000 /* 256M */
 #define MEM_SIZE_64M	(u32)0x4000000 /* 64M */
 
@@ -138,7 +140,8 @@ int BspDpaShmemSetup(void)
 	    printf("can not open mem!\n");
 	    return -1;
 	}
-	g_u8tmp1 = mmap((void *)0,BspGetUsDpaSize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(void *)BspGetUsDpaPhyBase());
+	g_u8tmp1 = mmap((void *)0,BspGetUsDpaSize(),PROT_READ|PROT_WRITE,MAP_SHARED,fd,(__off_t)BspGetUsDpaPhyBase());
+	g_u8tmp1 = g_u8tmp1;
 	close(fd);
 	#if 0
 	ret = mprotect((void *)g_u8tmp1, BspGetUsDpaSize(), PROT_READ | PROT_WRITE);
diff --git a/modules/spi/src/bsp_spi.c b/modules/spi/src/bsp_spi.c
index d9a1a35..a5796cd 100644
--- a/modules/spi/src/bsp_spi.c
+++ b/modules/spi/src/bsp_spi.c
@@ -219,7 +219,7 @@ s32 bsp_spi_setup_slave(u32 u32cs,u32 u32max_hz,u32 u32mode)
 *********************************************************************************/
 void bsp_spi_init(void)
 {
-	s32 i;
+	//s32 i;
 	
 	//printf("g_u8ccsbar = %x\n",g_u8ccsbar);
 	spi_reg	 = (struct fsl_espi_reg *)(g_u8ccsbar + 0x110000);  //定义寄存器地址
@@ -293,9 +293,8 @@ Private void spi_cs_deactivate(void)
 *********************************************************************************/
 s32 spi_write(u32 u32cs,u32 s32len, const void *pdata_out)
 {
-	u32 u32tmpdout, u32tmpdin, u32event;
+	u32 u32tmpdout, u32event;
 	s32 s32num_blks, s32num_chunks, s32max_tran_len, s32tran_len;
-	u8 *u8pch;
 	u8 *u8pbuffer = NULL;
 	u8 *dout = NULL;
 	s32 s32data_len = s32len;
@@ -372,7 +371,7 @@ s32 spi_write(u32 u32cs,u32 s32len, const void *pdata_out)
 s32 spi_read(u32 u32cs,u8 *cmd,u8 cmd_len,s32 s32len,u8 *pdata_in)
 {
 	u32 u32tmpdout, u32tmpdin, u32event;
-	s32 s32num_blks, s32num_chunks, s32max_tran_len, s32tran_len,s32cmd_blks,s32cmd_bytes;
+	s32 s32num_blks, s32num_chunks, s32max_tran_len, s32tran_len;
 	s32 s32num_bytes;
 	u8 *u8pch;
 	void *pdin;
@@ -597,7 +596,7 @@ s32 spi_cpld_read(u8 *cmd,u8 cmd_len,u8 *u8cpld_data,u32 len)
 		printf("Invalid device.\n");
 		return BSP_ERROR;
 	}
-	if(BSP_OK != bsp_spi_read(cmd,cmd_len,SPI_CS2,len,u8cpld_data))
+	if(BSP_OK != bsp_spi_read(SPI_CS2, cmd,cmd_len,len,u8cpld_data))
 	{
 		printf("cpld write wrong!\n");
 		return BSP_ERROR;
diff --git a/modules/usdpaa/inc/dcl.h b/modules/usdpaa/inc/dcl.h
index 29b23f1..3dfc108 100644
--- a/modules/usdpaa/inc/dcl.h
+++ b/modules/usdpaa/inc/dcl.h
@@ -205,7 +205,7 @@ uint32_t *cmd_insert_load(uint32_t *descwd, void *data,
 			   uint32_t dest, uint8_t offset,
 			   uint8_t len, enum item_inline imm);
 
-uint32_t *cmd_insert_fifo_load(uint32_t *descwd, uint64_t data,
+uint32_t *cmd_insert_fifo_load(uint32_t *descwd, uint8_t *data,
 				uint32_t len,
 				uint32_t class_access, uint32_t sgflag,
 				uint32_t imm, uint32_t ext, uint32_t type);
diff --git a/modules/usdpaa/inc/fm_pcd_ioctls.h b/modules/usdpaa/inc/fm_pcd_ioctls.h
index 135d71f..a28e9b2 100644
--- a/modules/usdpaa/inc/fm_pcd_ioctls.h
+++ b/modules/usdpaa/inc/fm_pcd_ioctls.h
@@ -1608,7 +1608,9 @@ static inline unsigned char EthId2PortDevId(e_EthId ucEthId)
 
 
 #define FM_PCD_IOC_CC_NODE_GET_CC_NODE_INFO _IOWR(FM_IOC_TYPE_BASE, FM_PCD_IOC_NUM(39), ioc_fm_CcNodeHandle_params_t)
+#if 0
 #define FM_PCD_IOC_CC_NODE_MODIFY_KEY       _IOW(FM_IOC_TYPE_BASE, FM_PCD_IOC_NUM(47), ioc_fm_pcd_cc_node_modify_key_params_t)
+#endif
 
 
 #define FM_PCD_IOC_CC_NODE_SET_CC_PORT_PARAMS_NODE_INFO     _IOWR(FM_IOC_TYPE_BASE, FM_PCD_IOC_NUM(48), t_FmPortParams)
diff --git a/modules/usdpaa/inc/fm_regs.h b/modules/usdpaa/inc/fm_regs.h
index fb49027..abab601 100644
--- a/modules/usdpaa/inc/fm_regs.h
+++ b/modules/usdpaa/inc/fm_regs.h
@@ -85,7 +85,7 @@
 #define FMBM_RDUC                   0x294
 #define FMBM_RFUC                   0x298
 #define FMBM_RPAC                   0x29c
-#define FMBM_PFS                    0x204
+//#define FMBM_PFS                    0x204
 
 #define FMBM_TRLMTS         0x002C/* Tx Rate Limiter Scale Register (FMBM_TRLMTS) */
 #define FMBM_TRLMT                          0x0030                               /* Tx Rate Limiter Register (FMBM_TRLMT) */
diff --git a/modules/usdpaa/inc/qman_low.h b/modules/usdpaa/inc/qman_low.h
index 9f85798..c824978 100644
--- a/modules/usdpaa/inc/qman_low.h
+++ b/modules/usdpaa/inc/qman_low.h
@@ -80,7 +80,6 @@ static inline void *ptr_OR(void *a, void *b)
 /* Cache-inhibited register access */
 static inline u32 __qm_in(struct qm_addr *qm, void *offset)
 {
-    u32 ret;
     //printf("&qm->addr_ci's addr-->0x%lx\n",qm->addr_ci);
     //printf("qm->addr_ci's value=0x%lx\n",*(unsigned int *)qm->addr_ci);
     //while(1);
diff --git a/modules/usdpaa/src/bman_driver.c b/modules/usdpaa/src/bman_driver.c
index 5fb7396..6843a95 100644
--- a/modules/usdpaa/src/bman_driver.c
+++ b/modules/usdpaa/src/bman_driver.c
@@ -9,7 +9,6 @@
 struct bm_portal gatBmPortal[10];
 struct bman_portal *gaptBmanPortal[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
 static __thread struct bm_portal portal;
-static __thread int fd;
 DEFINE_PER_CPU(struct bman_portal *, bman_affine_portal);
 
 u8 bm_portal_num(void)
@@ -101,7 +100,6 @@ int  BspBmanPortalInit(unsigned long  portalnum, int cpu)
 		/* FIXME: hard-coded */
 		.mask = BMAN_DEPLETION_FULL
 	};
-	unsigned long tmp;
 	struct bm_addr addr;
 	struct bm_portal *portal;
 
diff --git a/modules/usdpaa/src/bman_high.c b/modules/usdpaa/src/bman_high.c
index 387e450..f39070d 100644
--- a/modules/usdpaa/src/bman_high.c
+++ b/modules/usdpaa/src/bman_high.c
@@ -617,7 +617,6 @@ static noinline struct bm_rcr_entry *wait_rel_start(struct bman_portal *p,
 							u32 flags)
 {
 	struct bm_rcr_entry *rcr;
-	int ret = 0;
 	if (flags & BMAN_RELEASE_FLAG_WAIT_INT)
 	{
 		while(!(rcr = try_rel_start(p)))
@@ -992,7 +991,6 @@ inline int BspBmanAcquire(unsigned int bpid, struct bm_buffer *bufs,u8 num, stru
     struct bm_mc_command *mcc;
     struct bm_mc_result *mcr;
     int ret;
-    unsigned long dwaddr;
     pthread_mutex_lock(&(p->mux_mc));
     mcc = bm_mc_start(p->p);	
     mcc->acquire.bpid = bpid;
@@ -1038,10 +1036,7 @@ inline int BspBmanAcquire(unsigned int bpid, struct bm_buffer *bufs,u8 num, stru
 ************************************************************************/
 int BspBmanQuery(struct bm_mc_result *ptmcr, struct bman_portal *p)
 {
-    struct bm_mc_command *mcc;
     struct bm_mc_result *mcr;
-    int ret;
-    unsigned long dwaddr;
     struct bman_depletion tmp;
 	pthread_mutex_lock(&(p->mux_mc));
 
@@ -1049,6 +1044,8 @@ int BspBmanQuery(struct bm_mc_result *ptmcr, struct bman_portal *p)
     while (!(mcr = bm_mc_result(p->p)))
         cpu_relax();
     tmp = mcr->query.ds.state;
+    tmp = tmp;
+#if 0
 	struct {
 		u8 __reserved1[32];
 		/* "availability state" and "depletion state" */
@@ -1058,7 +1055,7 @@ int BspBmanQuery(struct bm_mc_result *ptmcr, struct bman_portal *p)
 			struct bman_depletion state;
 		} as, ds;
 	} query;
-
+#endif
  
      if(ptmcr != NULL)
     {
diff --git a/modules/usdpaa/src/bsp_usdpaa.c b/modules/usdpaa/src/bsp_usdpaa.c
index 2a2180d..0fdffff 100644
--- a/modules/usdpaa/src/bsp_usdpaa.c
+++ b/modules/usdpaa/src/bsp_usdpaa.c
@@ -118,6 +118,7 @@ T_IFPRTCL       gatIfPrtclTbl[MAX_PRTCL_NUM] =
     {""},
 };         
 
+extern int BspSendIfData(unsigned long ifBindTblIndex, char *pbyData, int dwLen);
 
 unsigned char MacStackRcvRtn(unsigned long ifBindTblIndex, char *buf, int len)
 {
@@ -185,7 +186,7 @@ unsigned char MacStackRcvRtn(unsigned long ifBindTblIndex, char *buf, int len)
 					 gatIfBindTbl[i].atProtocol[j].pReceiveReturn((unsigned char *) byData,dwDataLen);
                     // gatIfBindTbl[i].atProtocol[0].pReceiveReturn( (unsigned char *) byData,dwDataLen);
 				 }
-				 return;
+				 return 0;
 			 }
 		 }
 	 }
@@ -195,7 +196,7 @@ unsigned char MacStackRcvRtn(unsigned long ifBindTblIndex, char *buf, int len)
  }
  
  /*没有找到 */
- return;
+ return 0;
 
 
 }
@@ -207,7 +208,7 @@ void   cap_handle_task(int arg)
     unsigned long   len = 0;
     printf("arg=%d\n", arg);
     ifBindTblIndex = arg;
-	int i;
+	//int i;
     while (1)
     {
         /*查询线程退出条件 */
@@ -230,7 +231,7 @@ void   cap_handle_task(int arg)
         printf("recvfrom len=0x%lx\n",len);
 	    #endif
 	if (len !=0xffffffff)
-        MacStackRcvRtn(ifBindTblIndex, rbuff, len);
+        MacStackRcvRtn(ifBindTblIndex, (char *)rbuff, len);
     }
 }
 
@@ -263,14 +264,8 @@ CHAR *BSP_S_strcpy( CHAR *pcDst, WORD32 dwMaxSize, const CHAR *pcSrc )
 
 int BspBindIfMuxProInit(T_IF * ptIf)
 {
-    pthread_t       a_thread;
-    pthread_attr_t  attr;
-    struct sched_param parm;
-    void           *thread_result;
     int             res;
     int             i;
-    int             ret = 0;
-    void           *arg;
     struct sockaddr_ll sll;
     struct ifreq    ifstruct;
     CHAR          ifName[20] = { 0 };
@@ -285,7 +280,7 @@ int BspBindIfMuxProInit(T_IF * ptIf)
     {
         if (gatIfBindTbl[i].isBind)
         {
-            if ((0 == strcmp(gatIfBindTbl[i].tIf.name, ptIf->name)) &&
+            if ((0 == strcmp((char *)gatIfBindTbl[i].tIf.name, (char *)ptIf->name)) &&
                 (gatIfBindTbl[i].tIf.unit == ptIf->unit))
                 break;
         }
@@ -344,7 +339,7 @@ int BspBindIfMuxProInit(T_IF * ptIf)
                 gatIfBindTbl[i].isBind = 1;
                 //printf("gatIfBindTbl[%d]\n",i);
                 /*启动接收线程 */
-                res = pthread_create(&ptid, NULL, (FUNCPTR)cap_handle_task, i); 
+                res = pthread_create(&ptid, NULL, (void * (*)(void *))cap_handle_task, (void *)i); 
                 if (ERROR == res)
                 {
                     perror("tCapHandleTask");
@@ -382,16 +377,16 @@ int BspRegisterProtocol(T_PROTOCOL * ptProtocol)
 #if 1
     int             i, j, k;
     int             ret = BSP_SUCCESS;
-    if ((0 == strcmp(ptProtocol->name, "")) || (0 == ptProtocol->atFeature[0].ftLen)|| (0 == ptProtocol->pReceiveReturn))
+    if ((0 == strcmp((char *)ptProtocol->name, "")) || (0 == ptProtocol->atFeature[0].ftLen)|| (0 == ptProtocol->pReceiveReturn))
     {
         return GMAC_COMM_ERR_PARAMERR;
     }
 
     for (i = 0; i < MAX_PRTCL_NUM; i++)
     {
-        if (0 != strcmp(gatIfPrtclTbl[i].name, ""))
+        if (0 != strcmp((char *)gatIfPrtclTbl[i].name, ""))
         {
-            if (0 == strcmp(gatIfPrtclTbl[i].name, ptProtocol->name))
+            if (0 == strcmp((char *)gatIfPrtclTbl[i].name, (char *)ptProtocol->name))
             {
                 ret = BspIfBindInit(&(gatIfPrtclTbl[i].tIf));
                 if (BSP_SUCCESS != ret)
@@ -404,7 +399,7 @@ int BspRegisterProtocol(T_PROTOCOL * ptProtocol)
                     {
                         if (gatIfBindTbl[j].isBind)
                         {
-                            if ((0 == strcmp(gatIfBindTbl[j].tIf.name, gatIfPrtclTbl[i].tIf.name)) && (gatIfBindTbl[j].tIf.unit == gatIfPrtclTbl[i].tIf.unit))
+                            if ((0 == strcmp((char *)gatIfBindTbl[j].tIf.name, (char *)gatIfPrtclTbl[i].tIf.name)) && (gatIfBindTbl[j].tIf.unit == gatIfPrtclTbl[i].tIf.unit))
                                 break;
                         }
                     }
@@ -456,7 +451,6 @@ void BspEthSwPort1CallBack(unsigned char *pbyReceiveData, unsigned long dwDataLe
 {
     int             i;
 #if 1
-    UCHAR ucaTempMac[6] = {0};
     printf("loading fGmaccTestCallBack0!\n");
 	        for (i = 0; i < dwDataLen; i++)
         {
@@ -472,7 +466,6 @@ void BspEthSwPort2CallBack(unsigned char *pbyReceiveData, unsigned long dwDataLe
 {
 #if 1
     int             i;
-    UCHAR ucaTempMac[6] = {0};
     printf("loading fGmaccTestCallBack1!\n");
 	        for (i = 0; i < dwDataLen; i++)
         {
@@ -488,7 +481,6 @@ void BspEthDebugCallBack(unsigned char *pbyReceiveData, unsigned long dwDataLen)
 {
 #if 1
     int             i;
-    UCHAR ucaTempMac[6] = {0};
 	printf("loading fGmaccTestCallBack2!\n");
         for (i = 0; i < dwDataLen; i++)
         {
@@ -504,7 +496,6 @@ void BspEthCoreNetCallBack( unsigned char *pbyReceiveData, unsigned long dwDataL
 {
 #if 1
     int             i;
-    UCHAR ucaTempMac[6] = {0};
 	printf("loading fGmaccTestCallBack3!\n");
     for (i = 0; i < dwDataLen; i++)
     {
@@ -522,7 +513,7 @@ void BspRegisterRecvData(void)
     T_PROTOCOL	 *ptProtocol = &tProtocol;
 	
     memset(ptProtocol, 0, sizeof(T_PROTOCOL));
-    strcpy(ptProtocol->name, "ETHSW1");
+    strcpy((char *)ptProtocol->name, "ETHSW1");
 	ptProtocol->atFeature[0].ftOffset = 12;
 	ptProtocol->atFeature[0].ftLen = 2;
 	ptProtocol->atFeature[0].ftValue[0] = 0xaa;
@@ -531,7 +522,7 @@ void BspRegisterRecvData(void)
     BspRegisterProtocol(ptProtocol);
 	
     memset(ptProtocol, 0, sizeof(T_PROTOCOL));
-    strcpy(ptProtocol->name, "ETHSW2");
+    strcpy((char *)ptProtocol->name, "ETHSW2");
     ptProtocol->atFeature[0].ftOffset = 12;
     ptProtocol->atFeature[0].ftLen = 2;
 	ptProtocol->atFeature[0].ftValue[0] = 0xaa;
@@ -540,7 +531,7 @@ void BspRegisterRecvData(void)
     BspRegisterProtocol(ptProtocol);
 	   
     memset(ptProtocol, 0, sizeof(T_PROTOCOL));
-    strcpy(ptProtocol->name, "DEBUG");
+    strcpy((char *)ptProtocol->name, "DEBUG");
     ptProtocol->atFeature[0].ftOffset = 12;
     ptProtocol->atFeature[0].ftLen = 2;
 	ptProtocol->atFeature[0].ftValue[0] = 0xaa;
@@ -549,17 +540,17 @@ void BspRegisterRecvData(void)
     BspRegisterProtocol(ptProtocol);
 	
     memset(ptProtocol, 0, sizeof(T_PROTOCOL));
-    strcpy(ptProtocol->name, "CORENET");
+    strcpy((char *)ptProtocol->name, "CORENET");
     ptProtocol->atFeature[0].ftOffset = 12;
     ptProtocol->atFeature[0].ftLen = 2;
 	ptProtocol->atFeature[0].ftValue[0] = 0x89;
 	ptProtocol->atFeature[0].ftValue[1] = 0x06;
 	ptProtocol->pReceiveReturn = (FUNCPTR)BspEthCoreNetCallBack;
     BspRegisterProtocol(ptProtocol);
-    return BSP_SUCCESS;
+    return;
 }
 
-int BspSendEmacData(unsigned char *pProtocolName, char *pbyData, int dwLen)
+int BspSendEmacData(char *pProtocolName, char *pbyData, int dwLen)
 {
     int             i, j;
     int             ret = BSP_SUCCESS;
@@ -579,7 +570,7 @@ int BspSendEmacData(unsigned char *pProtocolName, char *pbyData, int dwLen)
             {
                 if (0 != gatIfBindTbl[i].atProtocol[j].name[0])
                 {
-                    if (0 == strcmp(pProtocolName, gatIfBindTbl[i].atProtocol[j].name))
+                    if (0 == strcmp((char *)pProtocolName, (char *)gatIfBindTbl[i].atProtocol[j].name))
                     {
                         ret = BspSendIfData(i, pbyData, dwLen);
                         /*统计 */
diff --git a/modules/usdpaa/src/bspbman.c b/modules/usdpaa/src/bspbman.c
index 6ed765f..9701de1 100644
--- a/modules/usdpaa/src/bspbman.c
+++ b/modules/usdpaa/src/bspbman.c
@@ -353,8 +353,8 @@ void BspUnCinvSetTlb0(int TlbSel,     unsigned long ulVirAddrStartL,    unsigned
 inline int size2bpid(int memtype, unsigned long size )
 {
     int i;
-	printf("gatBmPoolGrpCtl[memtype].initflag->0x%lx\n",gatBmPoolGrpCtl[memtype].initflag);
-	printf("gatBmPoolGrpCtl[memtype].poolcount->0x%lx\n",gatBmPoolGrpCtl[memtype].poolcount);
+	printf("gatBmPoolGrpCtl[memtype].initflag->0x%x\n",gatBmPoolGrpCtl[memtype].initflag);
+	printf("gatBmPoolGrpCtl[memtype].poolcount->0x%x\n",gatBmPoolGrpCtl[memtype].poolcount);
 	//if((memtype >= BM_MAX_POOLGRP_COUNT) || (!gatBmPoolGrpCtl[memtype].initflag))
 	//{
 	//	BspBmanPrintf("%s failed, in file:%s, on line:%d, memtype = %d, size = %ld\n", __FUNCTION__, __FILE__, __LINE__, memtype, size);
@@ -390,7 +390,7 @@ TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long d
 	int ret;
 	//TBufCtl *ptBufCtl = NULL;
 	int portalnum;
-    printf("gpuBmancvirtbase->0x%08x\n",gpuBmancvirtbase);
+    printf("gpuBmancvirtbase->0x%08x\n",(unsigned int)gpuBmancvirtbase);
 	if((dwMemType >= gatBmPoolGrpCount))
 	{
 		BspBmanPrintf("%s failed, in file:%s, on line:%d\n", __FUNCTION__, __FILE__, __LINE__);
@@ -408,7 +408,7 @@ TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long d
 	//printf("hello-11111111111111111111\n");
 	ret = BspBmanAcquire(bpid, &bufs,1, gaptBmanPortal[portalnum]);
 	g_bufslo = bufs.lo;
-	printf("g_bufslo->0x%lx\n",g_bufslo);
+	printf("g_bufslo->0x%x\n",(unsigned int)g_bufslo);
 //	printf("ret->0x%lx\n",ret);
 	if(ret != 1)
 	{
@@ -436,7 +436,7 @@ TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long d
 	   
 
 
-	printf("ptbuf---1->0x%lx\n",ptBuf);
+	printf("ptbuf---1->0x%p\n",ptBuf);
 	//ptBufCtl = (TBufCtl *)((unsigned long)ptBuf - sizeof(TBufCtl));
 	//printf("hello-444444444444444\n");
 
@@ -452,8 +452,8 @@ TBuf  *BspGetTBuf(unsigned long dwsize, unsigned long dwMemType, unsigned long d
 	ptBuf->pucEnd = ptBuf->pucData + dwsize;
 	ptBuf->wBpid = bpid;
 	//ptBuf->pucBufStart = ptBuf - UNIHEAD_SIZE_128BYTES;
-    printf("ptBuf->pucData->0x%lx\n",ptBuf->pucData);
-	printf("ptBuf->pucEnd->0x%lx\n",ptBuf->pucEnd);
+    printf("ptBuf->pucData->0x%p\n",ptBuf->pucData);
+	printf("ptBuf->pucEnd->0x%p\n",ptBuf->pucEnd);
 	*(ptBuf->pucEnd)  = BMAN_TBUF_CHECKBYTE;
 
     //printf("hello-55566666\n");
@@ -530,7 +530,7 @@ int BspBmanBufPoolInit(unsigned long  bpid, unsigned long bufcfgsize, unsigned l
 	int ret;
 	//unsigned char *pucvirtbase;
 	unsigned long phybase;
-	unsigned long totalsize;
+//	unsigned long totalsize;
 	unsigned long bufsize;
 	int portalnum = USDPAA_BMAN_PORTAL_NUM;
 	
@@ -763,9 +763,10 @@ int BspIpReasembleBufPoolInit(unsigned int bufcount)
 	dwtotalsize = BspAlignSize(0x100000, dwtotalsize);
 	
 
-	pucvirtbase  =  BspShmemVirtMalloc(0x100000, dwtotalsize, "IpReasemble", 0,&pucvirtbase);
+	pucvirtbase  =  BspShmemVirtMalloc(0x100000, dwtotalsize, "IpReasemble", 0,(int *)&pucvirtbase);
 
 	dwPhyBase = __shmem_vtop(pucvirtbase);
+	dwPhyBase = dwPhyBase;
 	ret = mprotect((void *)(pucvirtbase), BspAlignSize(0x100000, dwtotalsize), PROT_READ | PROT_WRITE);
 	if (0 != ret) {
 		BspBmanPrintf("can't mprotect() shmem device");
@@ -795,7 +796,6 @@ int BspBmanInitExample()
 {
     int ret;
 
-    int i, j;
     g_dwbspbmanFlag = __LINE__;
     ret = BspBmanPoolGrpInit(g_tBmPoolCfg, sizeof(g_tBmPoolCfg)/sizeof(TBmPoolCfg), tbuf_mem_type0, 0xff);
     if(0 != ret)
@@ -804,6 +804,8 @@ int BspBmanInitExample()
     	BspBmanPrintf("%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, ret);
     	return ret;
     }    
+
+    return 0;
 }
 void BspTBuffInit(void *pBuff, int bpid, unsigned int buffsize)
 {
diff --git a/modules/usdpaa/src/bspfman.c b/modules/usdpaa/src/bspfman.c
index ec473b9..d786752 100644
--- a/modules/usdpaa/src/bspfman.c
+++ b/modules/usdpaa/src/bspfman.c
@@ -39,6 +39,15 @@ char gaucPacket[] = {
 };
 #endif
 
+extern struct qman_fq  *BspQmFqForDeqInit(u32 fqid, enum qm_channel channel, 
+                                                     enum qm_wq wq, unsigned long qman_fq_flag, qman_cb_dqrr pfun);
+extern void qm_set_memory(enum qm_memory memory, u16 eba,
+			u32 ba, int enable, int prio, int stash, u32 exp);
+extern int qm_init_pfdr(u32 pfdr_start, u32 num);
+extern void qm_set_QCSP_BAR( u16 eba, u32 ba);
+extern  void d4 (u32 uladdr,u32 len);
+
+
 typedef struct tagGtpuPktStat
 {
 	unsigned long dwfdformaterr;
@@ -184,8 +193,6 @@ static enum qman_cb_dqrr_result cb_dqrr_gtpu_udp_rx(struct qman_portal *qm,
 {
     TBuf* ptBuf;
     TBufCtl *ptBufCtl = NULL;
-	struct bm_buffer buf;
-	int i, j;
       
     if(BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid) < 100)
     {
@@ -196,7 +203,7 @@ static enum qman_cb_dqrr_result cb_dqrr_gtpu_udp_rx(struct qman_portal *qm,
     if((dqrr->fd.format) != qm_fd_contig)
     {
         //g_tGtpuPktStat.dwfdformaterr;
-		g_tGtpuPktStat.dwfdformaterr;
+		//g_tGtpuPktStat.dwfdformaterr;
 	BspDropFrame(&(dqrr->fd));
 	BspDpaaPrintf("error, in file:%s, on line:%d\n", __FILE__, __LINE__);
 	return qman_cb_dqrr_consume;
@@ -218,7 +225,7 @@ static enum qman_cb_dqrr_result cb_dqrr_gtpu_udp_rx(struct qman_portal *qm,
 	if((BMAN_TBUFCTL_CHECKWORD != (ptBufCtl->checkword)) || ((ptBufCtl->bpid) != (ptBuf->wBpid)))
 	{
 		g_tGtpuPktStat.dwtbufheadcheckerr++;
-	   	BspDpaaPrintf("error, in file:%s, on line:%d, ptBufCtl->checkword = 0x%x, ptBuf->wBpid = %d\n", __FILE__, __LINE__, ptBufCtl->checkword, ptBuf->wBpid);
+	   	BspDpaaPrintf("error, in file:%s, on line:%d, ptBufCtl->checkword = 0x%x, ptBuf->wBpid = %d\n", __FILE__, __LINE__, (unsigned int)ptBufCtl->checkword, (unsigned int)ptBuf->wBpid);
 	}
 #endif
 
@@ -314,8 +321,8 @@ static enum qman_cb_dqrr_result CbDqrrEmacRx(struct qman_portal *qm,
 					struct qman_fq *fq,
 					const struct qm_dqrr_entry *dqrr)
 {
-	TBuf* ptBuf;
-	TBufCtl *ptBufCtl = NULL;
+	TBuf* ptBuf = NULL;
+	//TBufCtl *ptBufCtl = NULL;
 	
     printf("%s:recv packet, count= %d, in pool %d\n", __func__,  BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid), dqrr->fd.bpid);
 	if(BspBmGetCounter(e_BM_IM_COUNTERS_POOL_CONTENT, dqrr->fd.bpid) < 100)
@@ -537,7 +544,7 @@ int BspUpIpSend(TBuf* ptBuf)
 {
     struct qm_fd qmfd;
     int reval;
-    TBufCtl *ptBufCtl = NULL;
+    //TBufCtl *ptBufCtl = NULL;
 	#if 1
     if((ptBuf == NULL)  || ((ptBuf->pucEnd) == NULL)  || (BMAN_TBUF_CHECKBYTE != (*(ptBuf->pucEnd))) || (IP_MIN_SIZE >  (ptBuf->dwDataSize)) || (NULL == gptTx))
     {
@@ -614,18 +621,14 @@ extern unsigned long g_sharephyaddr;
 int BspEmacDataSendTest()
 {
 
-    struct bm_buffer buf;
-    struct qm_fd qmfd;
-    int tmp;
+//    struct qm_fd qmfd;
     int ret = 0;
-    unsigned char *pucPacket = NULL;
 
 	TBuf  *ptBuf = NULL;
     int i;
     int count;
-    struct qm_mcr_queryfq_np np;
-    int bmportalnum = USDPAA_BMAN_PORTAL_NUM;
-    int qmportalnum = USDPAA_QMAN_PORTAL_NUM;
+//    struct qm_mcr_queryfq_np np;
+    //int qmportalnum = USDPAA_QMAN_PORTAL_NUM;
     g_dwusdpaaflag = __LINE__;
    
 	while(1)//for(i = 0; i < 2; )   /*  网口报文发送测试 */
@@ -1055,76 +1058,78 @@ int BspBmiRegPrint(unsigned int  port, int fm)
 		return -1;
 	
 	BspDpaaPrintf("BMI Common Registers:\n");
-	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
-	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
-	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
-	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
-	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
-	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
+	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
+	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
+	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
+	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
+	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
+	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
 
 	BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
-	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
-	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
-	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
-	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
-	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
-	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
-	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
-	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
-	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
-	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
-	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
-	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
-	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
-	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
-	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
-	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
-	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
-	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
-	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
-	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
-
-
-
-	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
-	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
+	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
+	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
+	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
+	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
+	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
+	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
+	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
+	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
+	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
+	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
+	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
+	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
+	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
+	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
+	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
+	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
+	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
+	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
+	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
+
+
+
+	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
+	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
 
 
 
 	 
      //ETH_DESEC1
-	 BspDpaaPrintf("ETH_DESEC1->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RBYT))));
+	 BspDpaaPrintf("ETH_DESEC1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RBYT))));
      //ETH_DESEC2
-	 BspDpaaPrintf("ETH_DESEC2->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RBYT))));
+	 BspDpaaPrintf("ETH_DESEC2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RBYT))));
      //ETH_DESEC3
-	 BspDpaaPrintf("ETH_DESEC3->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RBYT))));
+	 BspDpaaPrintf("ETH_DESEC3->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RBYT))));
      //ETH_DESEC4
-	 BspDpaaPrintf("ETH_DESEC4->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RBYT))));
+	 BspDpaaPrintf("ETH_DESEC4->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RBYT))));
      //ETH_DESEC5
-	 BspDpaaPrintf("ETH_DESEC5->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RBYT))));
+	 BspDpaaPrintf("ETH_DESEC5->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RBYT))));
 
 
 	 
-	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));
+	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));
 
 	 
-	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
-
-	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
-	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
-	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
-	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
-	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
-	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
-	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
-	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
-
-     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
-     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
-     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
-     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
-     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
-     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
+	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
+
+	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
+	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
+	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
+	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
+	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
+	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
+	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
+	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
+
+     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
+     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
+     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
+     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
+     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
+     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
+
+     return 0;
 }
 
 int BspBmiRegSet(unsigned int  port, int fm)
@@ -1144,15 +1149,16 @@ int BspBmiRegSet(unsigned int  port, int fm)
 	BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
 	
 	
-	(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))) = 0x440000;/* next en parser */
-	BspDpaaPrintf("FMBM_RFNE:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
+	(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))) = 0x440000;/* next en parser */
+	BspDpaaPrintf("FMBM_RFNE:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
 
-	(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))) = 0x480000;
-	BspDpaaPrintf("FMBM_RFPNE:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
+	(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))) = 0x480000;
+	BspDpaaPrintf("FMBM_RFPNE:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
 	
-	(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))) = 0xD40000;
-	BspDpaaPrintf("FMBM_RFENE:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
+	(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))) = 0xD40000;
+	BspDpaaPrintf("FMBM_RFENE:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
 	
+	return 0;
 }
 
 
@@ -1179,40 +1185,40 @@ int BspParserRegShow(unsigned int  port)
 	Fm_Offset = CCSR_FM1_OFFSET;
 	
 	BspDpaaPrintf("Parser Port%d  Registers:\n", port);
-	BspDpaaPrintf("FMPR_PxCAC:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCAC))));
-	BspDpaaPrintf("FMPR_PxCTPID:reg->0x%lx ,0x%x\n", (unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCTPID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCTPID))));
+	BspDpaaPrintf("FMPR_PxCAC:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCAC))));
+	BspDpaaPrintf("FMPR_PxCTPID:reg->0x%lx ,0x%x\n", (unsigned long)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCTPID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + PARSER_OFFSET  + FMPR_PxCTPID))));
 
 
 
 	BspDpaaPrintf("Parser Global Configuration Registers:\n");
 	
-	BspDpaaPrintf("FMPR_PARSE_MEM:reg->0x%lx ,0x%x\n", (unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PARSE_MEM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PARSE_MEM))));
+	BspDpaaPrintf("FMPR_PARSE_MEM:reg->0x%lx ,0x%x\n", (unsigned long)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PARSE_MEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PARSE_MEM))));
 	
 		
-	BspDpaaPrintf("FMPR_PEVR:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PEVR))));
-	BspDpaaPrintf("FMPR_PEVER:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET +  FMPR_PEVER))));
-	BspDpaaPrintf("FMPR_PERR:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PERR))));
-	BspDpaaPrintf("FMPR_PERER:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PERER))));
-	BspDpaaPrintf("FMPR_PPSC:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PPSC))));
-	BspDpaaPrintf("FMPR_PDS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PDS))));
-
-    BspDpaaPrintf("FMPR_L2RRS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L2RRS))));
-    BspDpaaPrintf("FMPR_L3RRS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L3RRS))));
-    BspDpaaPrintf("FMPR_L4RRS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L4RRS))));
-    BspDpaaPrintf("FMPR_SRRS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SRRS))));
-    BspDpaaPrintf("FMPR_L2RRES:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L2RRES))));
-    BspDpaaPrintf("FMPR_L3RRES:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L3RRES))));
-    BspDpaaPrintf("FMPR_L4RRES:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L4RRES))));
-    BspDpaaPrintf("FMPR_SRRES:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SRRES))));
-    BspDpaaPrintf("FMPR_SPCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SPCS))));
-    BspDpaaPrintf("FMPR_SPSCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SPSCS))));
-    BspDpaaPrintf("FMPR_HXSCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_HXSCS))));
-    BspDpaaPrintf("FMPR_MRCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MRCS))));
-    BspDpaaPrintf("FMPR_MWCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MWCS))));
-    BspDpaaPrintf("FMPR_MRSCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MRSCS))));
-    BspDpaaPrintf("FMPR_MWSCS:0x%x\n", (*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MWSCS))));
+	BspDpaaPrintf("FMPR_PEVR:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PEVR))));
+	BspDpaaPrintf("FMPR_PEVER:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET +  FMPR_PEVER))));
+	BspDpaaPrintf("FMPR_PERR:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PERR))));
+	BspDpaaPrintf("FMPR_PERER:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PERER))));
+	BspDpaaPrintf("FMPR_PPSC:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PPSC))));
+	BspDpaaPrintf("FMPR_PDS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_PDS))));
+
+    BspDpaaPrintf("FMPR_L2RRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L2RRS))));
+    BspDpaaPrintf("FMPR_L3RRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L3RRS))));
+    BspDpaaPrintf("FMPR_L4RRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L4RRS))));
+    BspDpaaPrintf("FMPR_SRRS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SRRS))));
+    BspDpaaPrintf("FMPR_L2RRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L2RRES))));
+    BspDpaaPrintf("FMPR_L3RRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L3RRES))));
+    BspDpaaPrintf("FMPR_L4RRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_L4RRES))));
+    BspDpaaPrintf("FMPR_SRRES:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SRRES))));
+    BspDpaaPrintf("FMPR_SPCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SPCS))));
+    BspDpaaPrintf("FMPR_SPSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_SPSCS))));
+    BspDpaaPrintf("FMPR_HXSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_HXSCS))));
+    BspDpaaPrintf("FMPR_MRCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MRCS))));
+    BspDpaaPrintf("FMPR_MWCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MWCS))));
+    BspDpaaPrintf("FMPR_MRSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MRSCS))));
+    BspDpaaPrintf("FMPR_MWSCS:0x%x\n", (*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_PARSERGLOBAL_OFFSET + FMPR_MWSCS))));
 	
-
+	return 0;
 }
 
 /*
@@ -1232,59 +1238,60 @@ int BspQmiRegPrint(unsigned int  port, int fm)
 	
 	BspDpaaPrintf("QMI Common Registers:\n");
 	
-	BspDpaaPrintf("FMQMI_GC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_GC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_GC))));
-	BspDpaaPrintf("FMQMI_EIE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET  + FMQMI_EIE))));
-	BspDpaaPrintf("FMQMI_EIEN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIEN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIEN))));
-	BspDpaaPrintf("FMQMI_EIF->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIF),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIF))));
-	BspDpaaPrintf("FMQMI_IE->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET    + FMQMI_IE))));
-	BspDpaaPrintf("FMQMI_IEN->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IEN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET +  + FMQMI_IEN))));
+	BspDpaaPrintf("FMQMI_GC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_GC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_GC))));
+	BspDpaaPrintf("FMQMI_EIE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET  + FMQMI_EIE))));
+	BspDpaaPrintf("FMQMI_EIEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIEN))));
+	BspDpaaPrintf("FMQMI_EIF->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIF))));
+	BspDpaaPrintf("FMQMI_IE->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET    + FMQMI_IE))));
+	BspDpaaPrintf("FMQMI_IEN->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET +  + FMQMI_IEN))));
 
 
 
-    BspDpaaPrintf("FMQMI_IF->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQMI_IF),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_IF))));
-    BspDpaaPrintf("FMQM_GS->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQM_GS),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_GS))));
-    BspDpaaPrintf("FMQM_ETFC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_ETFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_ETFC))));
-    BspDpaaPrintf("FMQM_DTFC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTFC))));
-    BspDpaaPrintf("FMQM_DC0->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC0),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC0))));
-    BspDpaaPrintf("FMQM_DC1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC1))));
-    BspDpaaPrintf("FMQM_DC2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC2))));
-    BspDpaaPrintf("FMQM_DC3->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC3),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC3))));
-    BspDpaaPrintf("FMQM_TAPC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_TAPC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_TAPC))));
-    BspDpaaPrintf("FMQM_DMCVC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DMCVC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DMCVC))));
+    BspDpaaPrintf("FMQMI_IF->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQMI_IF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_IF))));
+    BspDpaaPrintf("FMQM_GS->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQM_GS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_GS))));
+    BspDpaaPrintf("FMQM_ETFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_ETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_ETFC))));
+    BspDpaaPrintf("FMQM_DTFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTFC))));
+    BspDpaaPrintf("FMQM_DC0->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC0),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC0))));
+    BspDpaaPrintf("FMQM_DC1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC1))));
+    BspDpaaPrintf("FMQM_DC2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC2))));
+    BspDpaaPrintf("FMQM_DC3->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC3),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC3))));
+    BspDpaaPrintf("FMQM_TAPC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_TAPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_TAPC))));
+    BspDpaaPrintf("FMQM_DMCVC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DMCVC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DMCVC))));
 
 
 
-    BspDpaaPrintf("FMQM_DIFDCC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + FMQM_DIFDCC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DIFDCC))));
-    BspDpaaPrintf("FMQM_DA1VC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DA1VC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DA1VC))));
-    BspDpaaPrintf("FMQM_DTRC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTRC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTRC))));
+    BspDpaaPrintf("FMQM_DIFDCC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + FMQM_DIFDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DIFDCC))));
+    BspDpaaPrintf("FMQM_DA1VC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DA1VC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DA1VC))));
+    BspDpaaPrintf("FMQM_DTRC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTRC))));
 
-    BspDpaaPrintf("FMQM_EFDDD->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_EFDDD),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_EFDDD))));
-    BspDpaaPrintf("FMQM_DTCA1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA1))));
-    BspDpaaPrintf("FMQM_DTVA1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA1))));
-    BspDpaaPrintf("FMQM_DTMA1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA1))));
+    BspDpaaPrintf("FMQM_EFDDD->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_EFDDD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_EFDDD))));
+    BspDpaaPrintf("FMQM_DTCA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA1))));
+    BspDpaaPrintf("FMQM_DTVA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA1))));
+    BspDpaaPrintf("FMQM_DTMA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA1))));
 
-    BspDpaaPrintf("FMQM_DTCA->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA))));
+    BspDpaaPrintf("FMQM_DTCA->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA))));
 
 
-    BspDpaaPrintf("FMQM_DTCA2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA2))));
-    BspDpaaPrintf("FMQM_DTVA2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA2))));
-    BspDpaaPrintf("FMQM_DTMA2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA2))));
+    BspDpaaPrintf("FMQM_DTCA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA2))));
+    BspDpaaPrintf("FMQM_DTVA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA2))));
+    BspDpaaPrintf("FMQM_DTMA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA2))));
 #if 0
     BspDpaaPrintf("QMI Rx Port%d  Registers:\n", port);
 	
-	BspDpaaPrintf("FMQM_PNC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET  + FMQM_PNC))));
-	BspDpaaPrintf("FMQM_PNS->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNS),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET  + FMQM_PNS))));
-
-    BspDpaaPrintf("FMQM_PNTS->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNTS),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNTS))));
-    BspDpaaPrintf("FMQM_PNEN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNEN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNEN))));
-    BspDpaaPrintf("FMQM_PNETFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNETFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNETFC))));
-    BspDpaaPrintf("FMQM_PNDN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDN))));
-    BspDpaaPrintf("FMQM_PNDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDC))));
-    BspDpaaPrintf("FMQM_PNDTFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDTFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDTFC))));
-    BspDpaaPrintf("FMQM_PNDFNOC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDFNOC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDFNOC))));
-    BspDpaaPrintf("FMQM_PNDCC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDCC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDCC))));
+	BspDpaaPrintf("FMQM_PNC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET  + FMQM_PNC))));
+	BspDpaaPrintf("FMQM_PNS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET  + FMQM_PNS))));
+
+    BspDpaaPrintf("FMQM_PNTS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNTS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNTS))));
+    BspDpaaPrintf("FMQM_PNEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNEN))));
+    BspDpaaPrintf("FMQM_PNETFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNETFC))));
+    BspDpaaPrintf("FMQM_PNDN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDN))));
+    BspDpaaPrintf("FMQM_PNDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDC))));
+    BspDpaaPrintf("FMQM_PNDTFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDTFC))));
+    BspDpaaPrintf("FMQM_PNDFNOC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDFNOC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDFNOC))));
+    BspDpaaPrintf("FMQM_PNDCC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDCC))));
 #endif
 #endif
+    return 0;
 }
 
 
@@ -1298,252 +1305,252 @@ void BspEthPortRegShow(void)
 	d4 ((CCSR_VIRTADDR_BASE + ETH_DESEC5),0x1000);
 	
 	BspDpaaPrintf("BMI Common Registers:\n");
-	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
-	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
-	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
-	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
-	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
-	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
-	BspDpaaPrintf("FMBM_GDE->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_GDE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_GDE))));
+	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
+	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
+	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
+	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
+	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
+	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
+	BspDpaaPrintf("FMBM_GDE->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_GDE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_GDE))));
 	for (i=1;i<64;i++)
 	{
-	    BspDpaaPrintf("FMBM_PP[%d]->0x%x value:0x%x\n", i,(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)))));
-	   // BspDpaaPrintf("FMBM_PFS[%d]->0x%x value:0x%x\n", i,(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)))));
-	    BspDpaaPrintf("FMBM_SPLIODN[%d]->0x%x value:0x%x\n", i,(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_SPLIODN(i)),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_SPLIODN(i)))));
+	    BspDpaaPrintf("FMBM_PP[%d]->0x%x value:0x%x\n", i,(unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)))));
+	   // BspDpaaPrintf("FMBM_PFS[%d]->0x%x value:0x%x\n", i,(unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)))));
+	    BspDpaaPrintf("FMBM_SPLIODN[%d]->0x%x value:0x%x\n", i,(unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_SPLIODN(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_SPLIODN(i)))));
 	}
     port =0xc;
     BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
-	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
-	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
-	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
-	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
-	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
-	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
-	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
-	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
-	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
-	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
-	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
-	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
-	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
-	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
-	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
-	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
-	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
-	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
-	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
-	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
-
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c))));
-
-
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c))));
-
-
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c))));
-
-
-	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
-	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
-
-	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));
+	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
+	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
+	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
+	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
+	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
+	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
+	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
+	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
+	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
+	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
+	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
+	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
+	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
+	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
+	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
+	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
+	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
+	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
+	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
+
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c))));
+
+
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c))));
+
+
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c))));
+
+
+	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
+	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
+
+	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));
 
 	 
-	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
+	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
 
-	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
-	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
-	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
-	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
-	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
-	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
-	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
-	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
+	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
+	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
+	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
+	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
+	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
+	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
+	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
+	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
 
-     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
-     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
-     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
-     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
-     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
-     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
+     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
+     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
+     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
+     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
+     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
+     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
 
 
 	
     port =0x2c;
     BspDpaaPrintf("BMI Tx Port%d  Registers:\n", port);
-	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
-	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
-	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
-	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
-	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
-	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
-	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
-	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
-	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
-	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
-	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
-	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
-	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
-	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
-	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
-	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
-	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
-	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
-	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
-	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
-
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c))));
-
-
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b))));
-	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c))));
-
-
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b))));
-	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c))));
-
-
-	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
-	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
-
-	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));
+	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
+	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
+	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
+	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
+	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
+	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
+	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
+	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
+	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
+	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
+	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
+	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
+	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
+	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
+	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
+	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
+	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
+	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
+	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
+
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x100))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x101))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x102))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x103))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x104))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x105))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x106))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x107))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x108))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x109))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10a))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10b))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x10c))));
+
+
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x110))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x111))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x112))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x113))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x114))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x115))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x116))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x117))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x118))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x119))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11a))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11b))));
+	 BspDpaaPrintf("FMBM_REBMPI->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x11c))));
+
+
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x120))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x121))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x122))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x123))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x124))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x125))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x126))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x127))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x128))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x129))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12a))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12b))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12c))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12d))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12e))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x12f))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x130))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x131))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x132))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x133))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x134))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x135))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x136))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x137))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x138))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x139))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13a))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13b))));
+	 BspDpaaPrintf("FMBM_RACNT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + 0x13c))));
+
+
+	 BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
+	 BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
+
+	 BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));
 
 	 
-	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
-
-	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
-	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
-	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
-	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
-	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
-	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
-	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
-	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
-
-     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
-     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
-     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
-     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
-     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
-     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
+	 BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
+
+	 BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
+	 BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
+	 BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
+	 BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
+	 BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
+	 BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
+	 BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
+	 BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
+
+     BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
+     BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
+     BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RRQUC))));
+     BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
+     BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
+     BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
 #if 0
-	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
-	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
+	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
+	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
 	//0x8    0x9	   0xa	  0xb	0xc 
     port = 0x8;
-	BspDpaaPrintf("RX0,FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("RX0,FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
     port = 0x9;
-	BspDpaaPrintf("RX1,FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("RX1,FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
     port = 0xa;
-	BspDpaaPrintf("RX2,FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("RX2,FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
     port = 0xc;
-	BspDpaaPrintf("RX3,FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("RX3,FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
 	for (i=1;i<64;i++)
 	{
-	    BspDpaaPrintf("FMBM_PP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)))));
-	    BspDpaaPrintf("FMBM_PFS->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)))));
+	    BspDpaaPrintf("FMBM_PP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_PP(i)))));
+	    BspDpaaPrintf("FMBM_PFS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_PFS(i)))));
 	}
 #endif
 #if 0
@@ -1565,149 +1572,148 @@ m4 0x4848f00c,0x3ff03ff
 void BspEthInfoShow(unsigned int  port)
 {
 #if 1
-    int i = 0;
 	unsigned long Fm_Offset;
 	Fm_Offset = CCSR_FM1_OFFSET;
     BspDpaaPrintf("MAC 1-5 INFO LIST!\n");
-	BspDpaaPrintf("DESEC1'RBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RBYT))));	
-	BspDpaaPrintf("DESEC1'RPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RPKT))));
-    BspDpaaPrintf("DESEC1'TBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_TBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_TBYT))));
-    BspDpaaPrintf("DESEC1'TPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_TPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_TPKT))));
+	BspDpaaPrintf("DESEC1'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RBYT))));	
+	BspDpaaPrintf("DESEC1'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_RPKT))));
+    BspDpaaPrintf("DESEC1'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_TBYT))));
+    BspDpaaPrintf("DESEC1'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC1+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC1+ETH_MAC_TPKT))));
 
 
-	BspDpaaPrintf("DESEC2'RBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RBYT))));
-	BspDpaaPrintf("DESEC2'RPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RPKT))));
-    BspDpaaPrintf("DESEC2'TBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_TBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_TBYT))));
-    BspDpaaPrintf("DESEC2'TPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_TPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_TPKT))));
+	BspDpaaPrintf("DESEC2'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RBYT))));
+	BspDpaaPrintf("DESEC2'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_RPKT))));
+    BspDpaaPrintf("DESEC2'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_TBYT))));
+    BspDpaaPrintf("DESEC2'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC2+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC2+ETH_MAC_TPKT))));
 
-	BspDpaaPrintf("DESEC3'RBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RBYT))));
-	BspDpaaPrintf("DESEC3'RPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RPKT))));
-    BspDpaaPrintf("DESEC3'TBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_TBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_TBYT))));
-    BspDpaaPrintf("DESEC3'TPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_TPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_TPKT))));
+	BspDpaaPrintf("DESEC3'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RBYT))));
+	BspDpaaPrintf("DESEC3'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_RPKT))));
+    BspDpaaPrintf("DESEC3'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_TBYT))));
+    BspDpaaPrintf("DESEC3'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC3+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC3+ETH_MAC_TPKT))));
 
-	BspDpaaPrintf("DESEC4'RBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RBYT))));
-	BspDpaaPrintf("DESEC4'RPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RPKT))));
-    BspDpaaPrintf("DESEC4'TBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_TBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_TBYT))));
-    BspDpaaPrintf("DESEC4'TPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_TPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_TPKT))));
+	BspDpaaPrintf("DESEC4'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RBYT))));
+	BspDpaaPrintf("DESEC4'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_RPKT))));
+    BspDpaaPrintf("DESEC4'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_TBYT))));
+    BspDpaaPrintf("DESEC4'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC4+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC4+ETH_MAC_TPKT))));
 
-	BspDpaaPrintf("DESEC5'RBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RBYT))));
-	BspDpaaPrintf("DESEC5'RPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RPKT))));
-    BspDpaaPrintf("DESEC5'TBYT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_TBYT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_TBYT))));
-    BspDpaaPrintf("DESEC5'TPKT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_TPKT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_TPKT))));
+	BspDpaaPrintf("DESEC5'RBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RBYT))));
+	BspDpaaPrintf("DESEC5'RPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_RPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_RPKT))));
+    BspDpaaPrintf("DESEC5'TBYT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_TBYT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_TBYT))));
+    BspDpaaPrintf("DESEC5'TPKT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE +ETH_DESEC5+ETH_MAC_TPKT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + ETH_DESEC5+ETH_MAC_TPKT))));
 
    
     
 	BspDpaaPrintf("BMI Common Registers:\n");
-	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
-	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
-	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
-	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
-	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
-	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
+	BspDpaaPrintf("FMBM_INIT->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_INIT))));
+	BspDpaaPrintf("FMBM_CFG1->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG1))));
+	BspDpaaPrintf("FMBM_CFG2->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_CFG2))));
+	BspDpaaPrintf("FMBM_IEVR->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IEVR))));
+	BspDpaaPrintf("FMBM_IER->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IER))));
+	BspDpaaPrintf("FMBM_IFR->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_IFR))));
 
 	BspDpaaPrintf("BMI Rx Port%d  Registers:\n", port);
-	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
-	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
-	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
-	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
-	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
-	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
-	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
-	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
-	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
-	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
-	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
-	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
-	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
-	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
-	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
-	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
-	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
-	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
-	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
-	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
-
-
-
-	BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
-	BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
-	BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFRC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));	 
-	BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RBFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
+	BspDpaaPrintf("FMBM_RCFG->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCFG))));
+	BspDpaaPrintf("FMBM_RST->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RST))));
+	BspDpaaPrintf("FMBM_RDA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDA))));
+	BspDpaaPrintf("FMBM_RFP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFP))));
+	BspDpaaPrintf("FMBM_RFED->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFED))));
+	BspDpaaPrintf("FMBM_RICP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RICP))));
+	BspDpaaPrintf("FMBM_RIM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RIM))));
+	BspDpaaPrintf("FMBM_REBM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REBM))));
+	BspDpaaPrintf("FMBM_RFNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFNE))));
+	BspDpaaPrintf("FMBM_RFCA->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFCA))));
+	BspDpaaPrintf("FMBM_RFPNE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFPNE))));
+	BspDpaaPrintf("FMBM_RPSO->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPSO))));
+	BspDpaaPrintf("FMBM_RPP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset +  port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPP))));
+	BspDpaaPrintf("FMBM_RCCB->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCB))));
+	BspDpaaPrintf("FMBM_RETH->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RETH))));
+	BspDpaaPrintf("FMBM_RFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFQID))));
+	BspDpaaPrintf("FMBM_REFQID->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_REFQID))));
+	BspDpaaPrintf("FMBM_RFSDM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSDM))));
+	BspDpaaPrintf("FMBM_RFSEM->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFSEM))));
+	BspDpaaPrintf("FMBM_RFENE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFENE))));
+
+
+
+	BspDpaaPrintf("FMBM_RMPD->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RMPD))));
+	BspDpaaPrintf("FMBM_RSTC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RSTC))));
+	BspDpaaPrintf("FMBM_RFRC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFRC))));	 
+	BspDpaaPrintf("FMBM_RBFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RBFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBFC))));
 	 
-	BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RLFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
-	BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
-	BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
-	BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
-	BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RODC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
-	BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RBDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
-	BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
-	BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPCP),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
+	BspDpaaPrintf("FMBM_RLFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RLFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RLFC))));
+	BspDpaaPrintf("FMBM_RFFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFFC))));
+	BspDpaaPrintf("FMBM_RFDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFDC))));
+	BspDpaaPrintf("FMBM_RFLDEC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFLDEC))));
+	BspDpaaPrintf("FMBM_RODC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RODC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RODC))));
+	BspDpaaPrintf("FMBM_RBDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RBDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RBDC))));
+	BspDpaaPrintf("FMBM_RPC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPC))));
+	BspDpaaPrintf("FMBM_RPCP->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPCP),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPCP))));
 	 
-	BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
-	BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
-	BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_RRQUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC))));
-	BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
-	BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
-	BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
+	BspDpaaPrintf("FMBM_RCCN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RCCN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RCCN))));
+	BspDpaaPrintf("FMBM_RTUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RTUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RTUC))));
+	BspDpaaPrintf("FMBM_RRQUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET + FMBM_RRQUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RRQUC))));
+	BspDpaaPrintf("FMBM_RDUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RDUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RDUC))));
+	BspDpaaPrintf("FMBM_RFUC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RFUC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RFUC))));
+	BspDpaaPrintf("FMBM_RPAC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET	+ FMBM_RPAC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + BMI_OFFSET  + FMBM_RPAC))));
 
     BspDpaaPrintf("QMI Common Registers:\n");
 
-	BspDpaaPrintf("FMQMI_GC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_GC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_GC))));
-	BspDpaaPrintf("FMQMI_EIE->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET  + FMQMI_EIE))));
-	BspDpaaPrintf("FMQMI_EIEN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIEN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIEN))));
-	BspDpaaPrintf("FMQMI_EIF->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIF),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIF))));
-	BspDpaaPrintf("FMQMI_IE->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IE),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET    + FMQMI_IE))));
-	BspDpaaPrintf("FMQMI_IEN->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IEN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET +  + FMQMI_IEN))));
+	BspDpaaPrintf("FMQMI_GC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_GC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_GC))));
+	BspDpaaPrintf("FMQMI_EIE->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET  + FMQMI_EIE))));
+	BspDpaaPrintf("FMQMI_EIEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIEN))));
+	BspDpaaPrintf("FMQMI_EIF->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_EIF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_EIF))));
+	BspDpaaPrintf("FMQMI_IE->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IE),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET    + FMQMI_IE))));
+	BspDpaaPrintf("FMQMI_IEN->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET +    FMQMI_IEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET +  + FMQMI_IEN))));
 
 
 
-    BspDpaaPrintf("FMQMI_IF->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQMI_IF),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_IF))));
-    BspDpaaPrintf("FMQM_GS->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQM_GS),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_GS))));
-    BspDpaaPrintf("FMQM_ETFC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_ETFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_ETFC))));
-    BspDpaaPrintf("FMQM_DTFC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTFC))));
-    BspDpaaPrintf("FMQM_DC0->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC0),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC0))));
-    BspDpaaPrintf("FMQM_DC1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC1))));
-    BspDpaaPrintf("FMQM_DC2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC2))));
-    BspDpaaPrintf("FMQM_DC3->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC3),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC3))));
-    BspDpaaPrintf("FMQM_TAPC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_TAPC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_TAPC))));
-    BspDpaaPrintf("FMQM_DMCVC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DMCVC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DMCVC))));
+    BspDpaaPrintf("FMQMI_IF->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQMI_IF),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQMI_IF))));
+    BspDpaaPrintf("FMQM_GS->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + 	 FMQM_GS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_GS))));
+    BspDpaaPrintf("FMQM_ETFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_ETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_ETFC))));
+    BspDpaaPrintf("FMQM_DTFC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTFC))));
+    BspDpaaPrintf("FMQM_DC0->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC0),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC0))));
+    BspDpaaPrintf("FMQM_DC1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC1))));
+    BspDpaaPrintf("FMQM_DC2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC2))));
+    BspDpaaPrintf("FMQM_DC3->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DC3),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DC3))));
+    BspDpaaPrintf("FMQM_TAPC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_TAPC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_TAPC))));
+    BspDpaaPrintf("FMQM_DMCVC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DMCVC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DMCVC))));
 
 
 
-    BspDpaaPrintf("FMQM_DIFDCC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + FMQM_DIFDCC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DIFDCC))));
-    BspDpaaPrintf("FMQM_DA1VC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DA1VC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DA1VC))));
-    BspDpaaPrintf("FMQM_DTRC->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTRC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTRC))));
+    BspDpaaPrintf("FMQM_DIFDCC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET + FMQM_DIFDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DIFDCC))));
+    BspDpaaPrintf("FMQM_DA1VC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DA1VC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DA1VC))));
+    BspDpaaPrintf("FMQM_DTRC->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTRC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTRC))));
 
-    BspDpaaPrintf("FMQM_EFDDD->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_EFDDD),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_EFDDD))));
-    BspDpaaPrintf("FMQM_DTCA1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA1))));
-    BspDpaaPrintf("FMQM_DTVA1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA1))));
-    BspDpaaPrintf("FMQM_DTMA1->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA1),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA1))));
+    BspDpaaPrintf("FMQM_EFDDD->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_EFDDD),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_EFDDD))));
+    BspDpaaPrintf("FMQM_DTCA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA1))));
+    BspDpaaPrintf("FMQM_DTVA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA1))));
+    BspDpaaPrintf("FMQM_DTMA1->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA1),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA1))));
 
-    BspDpaaPrintf("FMQM_DTCA->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA))));
+    BspDpaaPrintf("FMQM_DTCA->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA))));
 
 
-    BspDpaaPrintf("FMQM_DTCA2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA2))));
-    BspDpaaPrintf("FMQM_DTVA2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA2))));
-    BspDpaaPrintf("FMQM_DTMA2->0x%x value:0x%x\n",  (CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA2),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA2))));
+    BspDpaaPrintf("FMQM_DTCA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTCA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTCA2))));
+    BspDpaaPrintf("FMQM_DTVA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTVA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTVA2))));
+    BspDpaaPrintf("FMQM_DTMA2->0x%x value:0x%x\n",  (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + FM_BMIQMIPARSER_OFFSET 	+ FMQM_DTMA2),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + FM_BMIQMIPARSER_OFFSET   + FMQM_DTMA2))));
 
     BspDpaaPrintf("QMI Rx Port%d  Registers:\n", port);
 
-    BspDpaaPrintf("FMQM_PNC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNC))));
-    BspDpaaPrintf("FMQM_PNS->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNS),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNS))));
+    BspDpaaPrintf("FMQM_PNC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNC))));
+    BspDpaaPrintf("FMQM_PNS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNS))));
 
-    BspDpaaPrintf("FMQM_PNTS->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNTS),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNTS))));
-    BspDpaaPrintf("FMQM_PNEN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNEN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNEN))));
-    BspDpaaPrintf("FMQM_PNETFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	 + FMQM_PNETFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNETFC))));
-    BspDpaaPrintf("FMQM_PNDN->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDN),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDN))));
-    BspDpaaPrintf("FMQM_PNDC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDC))));
-    BspDpaaPrintf("FMQM_PNDTFC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	 + FMQM_PNDTFC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDTFC))));
-    BspDpaaPrintf("FMQM_PNDFNOC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDFNOC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDFNOC))));
-    BspDpaaPrintf("FMQM_PNDCC->0x%x value:0x%x\n", (CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	+ FMQM_PNDCC),(*((unsigned long*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDCC))));
+    BspDpaaPrintf("FMQM_PNTS->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNTS),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNTS))));
+    BspDpaaPrintf("FMQM_PNEN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNEN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNEN))));
+    BspDpaaPrintf("FMQM_PNETFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	 + FMQM_PNETFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNETFC))));
+    BspDpaaPrintf("FMQM_PNDN->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDN),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDN))));
+    BspDpaaPrintf("FMQM_PNDC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDC))));
+    BspDpaaPrintf("FMQM_PNDTFC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	 + FMQM_PNDTFC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDTFC))));
+    BspDpaaPrintf("FMQM_PNDFNOC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET   + FMQM_PNDFNOC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDFNOC))));
+    BspDpaaPrintf("FMQM_PNDCC->0x%x value:0x%x\n", (unsigned int)(CCSR_VIRTADDR_BASE+Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET	+ FMQM_PNDCC),(*((unsigned int*)(CCSR_VIRTADDR_BASE + Fm_Offset + port * 0x1000 + FM_BMIQMIPARSER_OFFSET + QMI_OFFSET	+ FMQM_PNDCC))));
 
 
     BspDpaaPrintf("BMAN   Registers:\n");
 
-    //BspDpaaPrintf( (*((unsigned long*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + BMAN_POOL0_CONTENT_OFFSET + (bpid <<2))));
+    //BspDpaaPrintf( (*((unsigned int*)(CCSR_VIRTADDR_BASE + BMAN_CCSR_OFFSET + BMAN_POOL0_CONTENT_OFFSET + (bpid <<2))));
 #endif
 }
 /**************************************************************************//**
@@ -1748,4 +1754,4 @@ fm_port_dual_rate_limiter_scale_down rate_limit_divider;    /**< For offline par
 
 #define DEVNAME_TOMCH0  "/dev/fm1-port-tx0"
 #define DEVNAME_TOMCH1  "/dev/fm1-port-tx1"
-                 
+
diff --git a/modules/usdpaa/src/bspqman.c b/modules/usdpaa/src/bspqman.c
index 6c76a48..378c2ca 100644
--- a/modules/usdpaa/src/bspqman.c
+++ b/modules/usdpaa/src/bspqman.c
@@ -23,8 +23,8 @@
 
 #define FQ_TAILDROP_THRESH 6400000
 
-
-/**********************************************************************
+extern void d4 (u32 uladdr,u32 len);
+/*********************************************************************
 * 函数名称：BspQmFqForDeqInit
 * 功能描述：软件出队的队列创建及初始化
 * 访问的表：无
@@ -90,7 +90,7 @@ struct qman_fq  *BspQmFqForDeqInit(u32 fqid, enum qm_channel channel,
 	      printf("BspQmFqForDeqInit--error2!\n");
 	      return NULL;
 	  }
-	  printf("gaptQmanPortal[%d]=%lx\n",portalnum,gaptQmanPortal[portalnum]);
+	  printf("gaptQmanPortal[%d]=%p\n",portalnum,gaptQmanPortal[portalnum]);
 	  
 	  ret = qman_init_fq(fq, QMAN_INITFQ_FLAG_SCHED, &opts, gaptQmanPortal[portalnum]);
 	  // printf("BspQmFqForDeqInit---oooooooooooo\n");
@@ -117,36 +117,37 @@ void BspShowFq(void)
 	if(fq == NULL)
 	{
 	    BspDpaaPrintf("file:%s, line:%d\n", __FILE__, __LINE__); 
-		return fq;
+		return ;
 	}
 	memset(fq, 0, sizeof(*fq));
     portalnum = USDPAA_QMAN_PORTAL_NUM;
     ret = qman_query_fq(fq, &fqd, gaptQmanPortal[portalnum]);
+    ret = ret;
 	
 	BspDpaaPrintf("fqd.td.exp = %x, fqd.td.mant = %x\n", fqd.td.exp, fqd.td.mant);
-	BspDpaaPrintf("fqd.orprws=0x%lx\n",fqd.orprws);
-	BspDpaaPrintf("fqd.oa=0x%lx\n",fqd.oa);
-	BspDpaaPrintf("fqd.olws=0x%lx\n",fqd.olws);
+	BspDpaaPrintf("fqd.orprws=0x%lx\n",(unsigned long)fqd.orprws);
+	BspDpaaPrintf("fqd.oa=0x%lx\n",(unsigned long)fqd.oa);
+	BspDpaaPrintf("fqd.olws=0x%lx\n",(unsigned long)fqd.olws);
 
 
-	BspDpaaPrintf("fqd.cgid=0x%lx\n",fqd.cgid);
+	BspDpaaPrintf("fqd.cgid=0x%lx\n",(unsigned long)fqd.cgid);
 
-	BspDpaaPrintf("fqd.fq_ctrl=0x%lx\n",fqd.fq_ctrl);
+	BspDpaaPrintf("fqd.fq_ctrl=0x%lx\n",(unsigned long)fqd.fq_ctrl);
 
-	BspDpaaPrintf("fqd.dest_wq=0x%lx\n",fqd.dest_wq);
+	BspDpaaPrintf("fqd.dest_wq=0x%lx\n",(unsigned long)fqd.dest_wq);
 
 
-	BspDpaaPrintf("fqd.dest.channel=0x%lx\n",fqd.dest.channel);
-    BspDpaaPrintf("fqd.dest.wq=0x%lx\n",fqd.dest.wq);
+	BspDpaaPrintf("fqd.dest.channel=0x%lx\n",(unsigned long)fqd.dest.channel);
+    BspDpaaPrintf("fqd.dest.wq=0x%lx\n",(unsigned long)fqd.dest.wq);
 	 
 		
-    BspDpaaPrintf("fqd.context_b=0x%lx\n",fqd.context_b);
+    BspDpaaPrintf("fqd.context_b=0x%lx\n",(unsigned long)fqd.context_b);
 
-	BspDpaaPrintf("fqd.context_a.hi=0x%lx\n",fqd.context_a.hi);
-	BspDpaaPrintf("fqd.context_a.lo=0x%lx\n",fqd.context_a.lo);
+	BspDpaaPrintf("fqd.context_a.hi=0x%lx\n",(unsigned long)fqd.context_a.hi);
+	BspDpaaPrintf("fqd.context_a.lo=0x%lx\n",(unsigned long)fqd.context_a.lo);
 
-	BspDpaaPrintf("fqd.context_a.context_hi=0x%lx\n",fqd.context_a.context_hi);
-	BspDpaaPrintf("fqd.context_a.context_lo=0x%lx\n",fqd.context_a.context_lo);
+	BspDpaaPrintf("fqd.context_a.context_hi=0x%lx\n",(unsigned long)fqd.context_a.context_hi);
+	BspDpaaPrintf("fqd.context_a.context_lo=0x%lx\n",(unsigned long)fqd.context_a.context_lo);
 	//fqd.context_b
 
 
@@ -285,26 +286,26 @@ void qm_set_QCSP_BAR( u16 eba,			u32 ba)
 void BspShowQmanRegInfo(void)
 {
 	printf("\r\n=====QMan Software Portal Configuration Registers====\r\n");
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET),0x100);
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET),0x100);
 	printf("\r\n=====Dynamic Debug(DD) Configuration Registers=======\r\n");
-	d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x200),0x20);
+	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x200),0x20);
 	printf("\r\n=====Direct Connect Portal(DCP) Configuration Registsers=\r\n");
-	d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x300),0x40);
+	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x300),0x40);
 	printf("\r\n======Packet Frame Descriptor Record(PFDR)Manager Query Registers=\r\n");
-	d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x400),0x10);
+	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x400),0x10);
 	printf("\r\n======Single Frame Descriptor Record(SFDR)Manager Register===\r\n");
-	d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x500),0x10);
+	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x500),0x10);
 	printf("\r\n======Work Queue Semaphore and Context Manager Register=====\r\n");
-	d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x600),0x100);
+	d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0x600),0x100);
 	printf("\r\n======Qman Error Capture Registers=========================\r\n");
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xa00),0x100);
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xa70),0x40);
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xa00),0x100);
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xa70),0x40);
 	printf("\r\n======Qman Initialization and Debug Control Registers===\r\n");
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xb00),0x40);
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xbe0),0x40);
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xb00),0x40);
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xbe0),0x40);
 	printf("\r\n======Qman Initiator Interface Memory Window Configuration Registers\r\n");
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xc00),0x40);  
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xd00),0x40);	
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xc00),0x40);  
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xd00),0x40);	
 	printf("\r\n======Qman Interrupt and Error Registers=======\r\n");
-    d4((unsigned long*)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xe00),0x40);
+    d4((unsigned long)(g_u8ccsbar + QMAN_CCSR_OFFSET+0xe00),0x40);
 }
diff --git a/modules/usdpaa/src/cmdgen.c b/modules/usdpaa/src/cmdgen.c
index 3dcd39b..9b8b874 100644
--- a/modules/usdpaa/src/cmdgen.c
+++ b/modules/usdpaa/src/cmdgen.c
@@ -858,7 +858,7 @@ uint32_t *cmd_insert_seq_load(uint32_t *descwd, uint32_t class_access,
  * @type   = FIFO input type, an OR combination of FIFOLD_TYPE_
  *           type and last/flush bits
  **/
-uint32_t *cmd_insert_fifo_load(uint32_t *descwd, uint64_t data,
+uint32_t *cmd_insert_fifo_load(uint32_t *descwd, uint8_t *data,
 				uint32_t len,
 				uint32_t class_access, uint32_t sgflag,
 				uint32_t imm, uint32_t ext, uint32_t type)
diff --git a/modules/usdpaa/src/fm_lib.c b/modules/usdpaa/src/fm_lib.c
index dec9194..41cfb74 100644
--- a/modules/usdpaa/src/fm_lib.c
+++ b/modules/usdpaa/src/fm_lib.c
@@ -150,7 +150,7 @@ t_Error FM_PCD_CcNodeModifyKey(t_Handle h_FmPcd, t_Handle h_CcNode, uint8_t keyI
 
     if (ioctl(p_Dev->fd, FM_PCD_IOC_CC_NODE_MODIFY_KEY, &params))
     {
-        printf("[usr debug] ioctl FM_PCD_IOC_CC_NODE_MODIFY_KEY p_Dev->fd:0x%x error\n");
+        printf("[usr debug] ioctl FM_PCD_IOC_CC_NODE_MODIFY_KEY p_Dev->fd:0x%x error\n", (unsigned int)p_Dev->fd);
         return (~0x0);
     }
 
diff --git a/modules/usdpaa/src/fsl_pcd_api.c b/modules/usdpaa/src/fsl_pcd_api.c
index 9aaa92e..6b8e5bf 100644
--- a/modules/usdpaa/src/fsl_pcd_api.c
+++ b/modules/usdpaa/src/fsl_pcd_api.c
@@ -1,8 +1,10 @@
 #include <string.h>
+#include <stdio.h>
 
 #include "fm_lib.h"
 #include "fsl_pcd_api.h"
 #include "fm_pcd_ioctls.h"
+#include "fm_pcd_ext.h"
 
 //unsigned char aucUnicastMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
 const unsigned char gaucUnicastMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
@@ -89,6 +91,15 @@ const unsigned char gaucUniArpMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] =
     },
 };
 
+extern t_Handle FM_Open(uint8_t id);
+extern 
+t_Handle FM_PCD_GetCcNodeInfo(t_Handle h_FmPcd, uint8_t ucFManDevId, uint8_t ucPortDevId, char *acDestName, uint8_t ucCcChainLevel, uint8_t *p_ucRouteIndexBase);
+extern 
+t_Error FM_PCD_CcNodeModifyKey(t_Handle h_FmPcd, t_Handle h_CcNode, uint8_t keyIndex, uint8_t keySize, uint8_t  *p_Key, uint8_t *p_Mask);
+extern 
+void FM_Close(t_Handle h_Fm);
+extern unsigned long BspGetDataNetMac(unsigned char * pucAddr);
+extern void bsp_sys_msdelay(unsigned long dwTimeOut);
 
 /* no vlan unicast arp reply and unicast arp request */
 const unsigned char aucMatchNonMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
@@ -146,7 +157,7 @@ int  BspModifyRouteKey(e_EthId  ucEthId, \
     uint8_t  ucRouteIndexBase;
 
     ucFManDevId = EthId2FManDevId(ucEthId);
-	printf("hahahahhahahahah ucFManDevId->0x%lx\n",ucFManDevId);
+	printf("hahahahhahahahah ucFManDevId->0x%lx\n",(unsigned long)ucFManDevId);
     if(0xff == ucFManDevId)
     {
         return ~0x0;
@@ -184,7 +195,7 @@ int  BspModifyRouteKey(e_EthId  ucEthId, \
         printf ("get CcNode info OK\n");
     }
 
-    ret = FM_PCD_CcNodeModifyKey(h_engines, h_CcNodeHandle, ucRouteIndexBase + ucRelativeIndex, 42, pKey, pMask);
+    ret = FM_PCD_CcNodeModifyKey(h_engines, h_CcNodeHandle, ucRouteIndexBase + ucRelativeIndex, 42, (unsigned char *)pKey, (unsigned char *)pMask);
     if (ret != 0)
     {
         printf ("Failed to modify key at runtime!, error = %d\r\n", ret);
@@ -212,8 +223,8 @@ int ChangeRoutToCcMaster(uint16_t wMasterCcBit)
                        "To CC_0", \
                        0, \
                        42,  \
-                       aucMatchAllMaskAndKey[1], \
-                       aucMatchAllMaskAndKey[0]);
+                       (char *)aucMatchAllMaskAndKey[1], \
+                       (char *)aucMatchAllMaskAndKey[0]);
         }
     }
     else if(0x4 == (wMasterCcBit&0x6))
@@ -224,24 +235,22 @@ int ChangeRoutToCcMaster(uint16_t wMasterCcBit)
                        "To CC_0", \
                        0, \
                        42,  \
-                       aucMatchNonMaskAndKey[1], \
-                       aucMatchNonMaskAndKey[0]);
+                       (char *)aucMatchNonMaskAndKey[1], \
+                       (char *)aucMatchNonMaskAndKey[0]);
         }
     }
     else
     {
         printf("[debug] error MasterCcBit:0x%x\n",wMasterCcBit);
     }
+
+    return 0;
 }
 
 
 int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr)
 {
     uint32_t ucLoop;
-    unsigned char *pMacH4Byte;
-    unsigned char *pMacL2Byte;
-    uint32_t dwMacH4Byte;
-    uint32_t dwMacL2Byte;
     uint32_t dwRet;
     unsigned char ucMacAddr[6] = {0};
     unsigned char ucIpAddr[4]  = {0};
@@ -281,8 +290,8 @@ int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr)
                    auDestName, \
                    0, \
                    42,  \
-                   aucUnicastMaskAndKeyTemp[1], \
-                   aucUnicastMaskAndKeyTemp[0]);
+                   (char *)aucUnicastMaskAndKeyTemp[1], \
+                   (char *)aucUnicastMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for Unicast, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -292,8 +301,8 @@ int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr)
                    auDestName, \
                    1, \
                    42,  \
-                   aucMultiArpMaskAndKeyTemp[1], \
-                   aucMultiArpMaskAndKeyTemp[0]);
+                   (char *)aucMultiArpMaskAndKeyTemp[1], \
+                   (char *)aucMultiArpMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for MultiArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -303,8 +312,8 @@ int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr)
                    auDestName, \
                    2, \
                    42,  \
-                   aucUniArpMaskAndKeyTemp[1], \
-                   aucUniArpMaskAndKeyTemp[0]);
+                   (char *)aucUniArpMaskAndKeyTemp[1], \
+                   (char *)aucUniArpMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for UniArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -318,10 +327,6 @@ int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr)
 int ModifyRoutForDsp(uint8_t ucDspId, uint8_t* pMacAddr)
 {
     uint32_t ucLoop;
-    unsigned char *pMacH4Byte;
-    unsigned char *pMacL2Byte;
-    uint32_t dwMacH4Byte;
-    uint32_t dwMacL2Byte;
     uint32_t dwRet;
     unsigned char ucMacAddr[6] = {0};
     unsigned char ucIpAddr[4]  = {0};
@@ -364,8 +369,8 @@ int ModifyRoutForDsp(uint8_t ucDspId, uint8_t* pMacAddr)
                    auDestName, \
                    0, \
                    42,  \
-                   aucUnicastMaskAndKeyTemp[1], \
-                   aucUnicastMaskAndKeyTemp[0]);
+                   (char *)aucUnicastMaskAndKeyTemp[1], \
+                   (char *)aucUnicastMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for Unicast, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -374,8 +379,8 @@ int ModifyRoutForDsp(uint8_t ucDspId, uint8_t* pMacAddr)
                    auDestName, \
                    1, \
                    42,  \
-                   aucMultiArpMaskAndKeyTemp[1], \
-                   aucMultiArpMaskAndKeyTemp[0]);
+                   (char *)aucMultiArpMaskAndKeyTemp[1], \
+                   (char *)aucMultiArpMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for MultiArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -384,8 +389,8 @@ int ModifyRoutForDsp(uint8_t ucDspId, uint8_t* pMacAddr)
                    auDestName, \
                    2, \
                    42,  \
-                   aucUniArpMaskAndKeyTemp[1], \
-                   aucUniArpMaskAndKeyTemp[0]);
+                   (char *)aucUniArpMaskAndKeyTemp[1], \
+                   (char *)aucUniArpMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for UniArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -433,8 +438,8 @@ int ModifyRoutForUsDpaa(uint8_t* pMacAddr, uint32_t ucUsDpaaIp, uint16_t wUdpPor
                    auDestName, \
                    0, \
                    42,  \
-                   aucUnicastMaskAndKeyTemp[1], \
-                   aucUnicastMaskAndKeyTemp[0]);
+                   (char *)aucUnicastMaskAndKeyTemp[1], \
+                   (char *)aucUnicastMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for Unicast, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -443,8 +448,8 @@ int ModifyRoutForUsDpaa(uint8_t* pMacAddr, uint32_t ucUsDpaaIp, uint16_t wUdpPor
                    auDestName, \
                    1, \
                    42,  \
-                   aucMultiArpMaskAndKeyTemp[1], \
-                   aucMultiArpMaskAndKeyTemp[0]);
+                   (char *)aucMultiArpMaskAndKeyTemp[1], \
+                   (char *)aucMultiArpMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for MultiArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
@@ -453,8 +458,8 @@ int ModifyRoutForUsDpaa(uint8_t* pMacAddr, uint32_t ucUsDpaaIp, uint16_t wUdpPor
                    auDestName, \
                    2, \
                    42,  \
-                   aucUniArpMaskAndKeyTemp[1], \
-                   aucUniArpMaskAndKeyTemp[0]);
+                   (char *)aucUniArpMaskAndKeyTemp[1], \
+                   (char *)aucUniArpMaskAndKeyTemp[0]);
         if(0 != dwRet)
             printf("[debug] BspModifyRouteKey() error for UniArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);
 
diff --git a/modules/usdpaa/src/jobdesc.c b/modules/usdpaa/src/jobdesc.c
index 5c09aa5..6653e2f 100644
--- a/modules/usdpaa/src/jobdesc.c
+++ b/modules/usdpaa/src/jobdesc.c
@@ -133,6 +133,7 @@ int cnstr_jobdesc_blkcipher_cbc(uint32_t *descbuf, uint16_t *bufsz,
 		memset(start, 0, (*bufsz * sizeof(uint32_t)));
 
 	startidx = descbuf - start;
+	startidx = startidx;
 	descbuf = cmd_insert_seq_in_ptr(descbuf, data_in, datasz, PTR_DIRECT);
 
 	descbuf = cmd_insert_seq_out_ptr(descbuf, data_out, datasz, PTR_DIRECT);
@@ -210,6 +211,7 @@ int cnstr_jobdesc_mdsplitkey(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++;
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	/* Pick key length from cipher submask as an enum */
 	keylen = mdkeylen[(cipher & OP_ALG_ALGSEL_SUBMASK) >>
@@ -231,7 +233,7 @@ int cnstr_jobdesc_mdsplitkey(uint32_t *descbuf, uint16_t *bufsize,
 				    ICV_CHECK_OFF, DIR_DECRYPT);
 
 	/* FIFO load of 0 to kickstart MDHA (this will generate pads) */
-	descbuf = cmd_insert_fifo_load(descbuf, NULL, 0, LDST_CLASS_2_CCB,
+	descbuf = cmd_insert_fifo_load(descbuf, 0, 0, LDST_CLASS_2_CCB,
 				       0, FIFOLD_IMM, 0,
 				       (FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST2));
 
@@ -281,6 +283,7 @@ int cnstr_jobdesc_snow_f8(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -337,6 +340,7 @@ int cnstr_jobdesc_snow_f8_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start; 
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -395,6 +399,7 @@ int cnstr_jobdesc_snow_f9(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 	                                                KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -455,6 +460,7 @@ int cnstr_jobdesc_snow_f9_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 	                                                KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -542,6 +548,7 @@ int32_t cnstr_jobdesc_lte_snow_f8_f9(uint32_t *descbuf, uint16_t *bufsize,
     descbuf += sizeof(struct snow_f8_f9_encap_pdb) >> 2;
 
     startidx = descbuf - start;
+	startidx = startidx;
 
     /*
     * Insert an empty instruction for a shared-JUMP past the keys
@@ -602,6 +609,7 @@ int32_t cnstr_jobdesc_lte_aes(uint32_t *descbuf, uint16_t *bufsize,
     descbuf += pdp_len >> 2;
 
     startidx = descbuf - start;
+	startidx = startidx;
 
     /*
     * Insert an empty instruction for a shared-JUMP past the keys
@@ -654,6 +662,7 @@ int cnstr_jobdesc_aes_ctr(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -698,6 +707,7 @@ int cnstr_jobdesc_aes_ctr_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -739,6 +749,7 @@ int cnstr_jobdesc_aes_cmac_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
 				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
@@ -785,6 +796,7 @@ int cnstr_jobdesc_sec_copy(uint32_t *descbuf, uint16_t *bufsize, uint8_t *in, ui
 
 	start = descbuf++; 
 	startidx = descbuf - start;
+	startidx = startidx;
 
 	//copy data from infifo to outfifo
 	descbuf = cmd_insert_move(descbuf, 1, MOVE_SRC_INFIFO,
@@ -814,6 +826,7 @@ int cnstr_jobdesc_sec_copy_sg(uint32_t *descbuf, uint16_t *bufsize, uint8_t *in,
     
     start = descbuf++; 
     startidx = descbuf - start;
+	startidx = startidx;
     
     //copy data from infifo to outfifo
     descbuf = cmd_insert_move(descbuf, 1, MOVE_SRC_INFIFO,
diff --git a/modules/usdpaa/src/qman_driver.c b/modules/usdpaa/src/qman_driver.c
index 20fbc1d..5fe1401 100644
--- a/modules/usdpaa/src/qman_driver.c
+++ b/modules/usdpaa/src/qman_driver.c
@@ -42,7 +42,6 @@ u16 qman_ip_rev = QMAN_REV1;
 struct qm_portal gatQmPortal[10];
 struct qman_portal *gaptQmanPortal[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
 static __thread struct qm_portal portal;
-static __thread int fd;
 DEFINE_PER_CPU(struct qman_portal *, qman_affine_portal);
 
 u8 qm_portal_num(void)
@@ -165,7 +164,6 @@ int  BspQmanPortalInit(unsigned long  portalnum, int cpu)
 		    /* FIXME: hard-coded */
 		    .has_hv_dma = 1
     };
-	unsigned long tmp;
 	struct qm_addr addr;
 	struct qm_portal *portal; 
 	int  fd_usdpaa = -1;
@@ -257,7 +255,7 @@ int  BspQmanPortalInit(unsigned long  portalnum, int cpu)
 		    /* default: enable all (available) pool channels */
 		    //qman_static_dequeue_add_ex(portal, ~0);
         #endif
-		    printf("Qman portal %d auto-initialised\n", portalnum);
+		    printf("Qman portal %d auto-initialised\n", (int)portalnum);
     }
 #endif
 
diff --git a/modules/usdpaa/src/qman_high.c b/modules/usdpaa/src/qman_high.c
index 13fe855..738763a 100644
--- a/modules/usdpaa/src/qman_high.c
+++ b/modules/usdpaa/src/qman_high.c
@@ -1340,7 +1340,7 @@ int qman_init_fq(struct qman_fq *fq, u32 flags, struct qm_mcc_initfq *opts, stru
 	  res = mcr->result;
 	if (res != QM_MCR_RESULT_OK) 
 	{ 
-	    printf("res->0x%lx\n",res);
+	    printf("res->0x%lx\n",(unsigned long)res);
 		FQUNLOCK(fq);
 		local_irq_enable();
 		put_affine_portal();
@@ -1797,7 +1797,6 @@ static noinline struct qm_eqcr_entry *wait_eq_start(struct qman_portal *p,
 							u32 flags)
 {
 	struct qm_eqcr_entry *eq;
-	int ret = 0;
 	if (flags & QMAN_ENQUEUE_FLAG_WAIT_INT)
 	{
 		while(!((eq = __try_eq(p))))
diff --git a/modules/usdpaa/src/qman_utility.c b/modules/usdpaa/src/qman_utility.c
index d1cf16e..019a108 100644
--- a/modules/usdpaa/src/qman_utility.c
+++ b/modules/usdpaa/src/qman_utility.c
@@ -101,6 +101,7 @@ int qman_fqid_pool_alloc(struct qman_fqid_pool *pool, u32 *fqid)
 		return -ENOMEM;
 	*fqid = pool->fqid_base + pool->next;
 	ret = test_and_set_bit(pool->next, pool->bits);
+	ret = ret;
 	BUG_ON(ret);
 	if (++pool->used == pool->total)
 		return 0;
@@ -119,6 +120,7 @@ void qman_fqid_pool_free(struct qman_fqid_pool *pool, u32 fqid)
 	fqid -= pool->fqid_base;
 	ret = test_and_clear_bit(fqid, pool->bits);
 	BUG_ON(!ret);
+	ret = ret;
 	if (pool->used-- == pool->total)
 		pool->next = fqid;
 }
diff --git a/modules/usdpaa/src/ringbuf.c b/modules/usdpaa/src/ringbuf.c
index 65137fa..c1ef4a3 100644
--- a/modules/usdpaa/src/ringbuf.c
+++ b/modules/usdpaa/src/ringbuf.c
@@ -9,6 +9,7 @@
 #include <semaphore.h>
 #include <signal.h>
 #include <dirent.h>
+#include <string.h>
 
 
 #include "bsp_usdpaa_ext.h"
@@ -37,8 +38,8 @@ Queue *InitQueue(void)
     memset(q->sizelen,0,MAXSIZE);
 	for(i=0;i<MAXSIZE;i++)
 	{
-	    q->qu[i]=(unsigned char *)malloc(MAX_BUF_LEN * sizeof(unsigned char));
-		if (q->qu[i] ==NULL)
+	    q->qu[i]=(unsigned int)malloc(MAX_BUF_LEN * sizeof(unsigned char));
+		if (q->qu[i] == 0)
 		{
 			return NULL;
 		}
@@ -148,16 +149,15 @@ void Display(void)
     printf("\n");
 #endif
     //printf("%4d",q->qu[(q->front+i)%MAXSIZE]);
-    printf("pQueue->front->0x%lx\n",pQueue->front);
-    printf("pQueue->rear->0x%lx\n",pQueue->rear);
-    printf("pQueue->tag->0x%lx\n",pQueue->tag);
+    printf("pQueue->front->0x%lx\n",(unsigned long)pQueue->front);
+    printf("pQueue->rear->0x%lx\n",(unsigned long)pQueue->rear);
+    printf("pQueue->tag->0x%lx\n",(unsigned long)pQueue->tag);
 }
 
 void buftest()
 {
     int icnt=0;
     Queue *q;
-	int c;
 	unsigned char x[8*1024];
 	int len;
 	int ret=0;
@@ -171,11 +171,11 @@ void buftest()
 	for (icnt=0;icnt<150;icnt++)
 	{
 		#if 1
-		ret = DeQueue(q,&len);
+		ret = DeQueue(q,(unsigned int *)&len);
 	    if(!ret)
 		    printf("empty!\n");
 		else
-		    printf("ret value->0x%lx,len->0x%lx\n",ret,len);
+		    printf("ret value->0x%lx,len->0x%lx\n",(unsigned long)ret,(unsigned long)len);
 		#endif
 	}	
 }
diff --git a/modules/usdpaa/src/secapp.c b/modules/usdpaa/src/secapp.c
index 73cda96..f9c4679 100644
--- a/modules/usdpaa/src/secapp.c
+++ b/modules/usdpaa/src/secapp.c
@@ -77,6 +77,59 @@ UCHAR snow_enc_f8_f9_reference_plaintext[][100] = {
                } 
 };
 
+extern int cnstr_jobdesc_snow_f8(uint32_t *descbuf, uint16_t *bufsize,
+			    uint8_t *key, uint32_t keylen,
+			    enum algdir dir,  uint32_t *ctx,
+			    uint8_t *in, uint8_t *out, uint16_t size);
+extern
+int cnstr_jobdesc_aes_ctr(uint32_t *descbuf, uint16_t *bufsize,
+						    uint8_t *key, uint32_t keylen,
+						    enum algdir dir,  uint32_t *ctx,
+						    uint8_t *in, uint8_t *out, uint16_t size);
+extern 
+int cnstr_jobdesc_sec_copy(uint32_t *descbuf, uint16_t *bufsize, uint8_t *in, uint8_t *out, uint16_t size);
+
+extern int cnstr_jobdesc_snow_f8_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
+			    uint8_t *key, uint32_t keylen,
+			    enum algdir dir,  uint32_t *ctx,
+			    uint8_t *in, uint8_t *out, uint16_t size);
+extern int cnstr_jobdesc_aes_ctr_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
+						    uint8_t *key, uint32_t keylen,
+						    enum algdir dir,  uint32_t *ctx,
+						    uint8_t *in, uint8_t *out, uint16_t size);
+extern 
+int cnstr_jobdesc_sec_copy_sg(uint32_t *descbuf, uint16_t *bufsize, uint8_t *in, uint8_t *out, uint16_t size);
+extern int cnstr_jobdesc_snow_f9(uint32_t *descbuf, uint16_t *bufsize,
+                                                   uint8_t *key, uint32_t keylen,
+                                                    enum algdir dir, uint32_t *ctx,
+                                                    uint8_t *in, uint16_t size, uint8_t *mac);
+extern int cnstr_jobdesc_snow_f9(uint32_t *descbuf, uint16_t *bufsize,
+                                                   uint8_t *key, uint32_t keylen,
+                                                    enum algdir dir, uint32_t *ctx,
+                                                    uint8_t *in, uint16_t size, uint8_t *mac);
+extern int32_t cnstr_jobdesc_lte_snow_f8_f9(uint32_t *descbuf, uint16_t *bufsize,
+                                                                    struct snow_f8_f9_encap_pdb *pdb,
+                                                                    struct cipherparams *cipherdata,
+                                                                    struct authparams *authdata, uint8_t dir);
+extern  int32_t cnstr_jobdesc_lte_aes(uint32_t *descbuf, uint16_t *bufsize,
+	                                struct snow_f8_f9_encap_pdb *pdb,
+	                                struct cipherparams *cipherdata,
+	                                struct authparams *authdata, uint8_t dir);
+
+extern  int cnstr_jobdesc_aes_cmac_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
+						    uint8_t *key, uint32_t keylen,
+						    enum algdir dir,
+						    uint8_t *in, uint16_t size, uint8_t *mac);
+extern int cnstr_jobdesc_snow_f9_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
+                                                   uint8_t *key, uint32_t keylen,
+                                                    enum algdir dir, uint32_t *ctx,
+                                                    uint8_t *in, uint16_t size, uint8_t *mac);
+
+extern unsigned long bsp_shm_v2p(void *pvaddr);
+extern unsigned long bsp_sec_reg_init(void);
+extern void dpadelay(unsigned int count);
+extern 
+void DumpSecReg(void);
 
 void pktwr_dump(char *comments, uint8_t *pkt, uint16_t length)
 {
@@ -111,11 +164,13 @@ void pktwr_dump(char *comments, uint8_t *pkt, uint16_t length)
  \Brief Enqueue Rejection Handler
  \param[out] msg Message Ring Entry
  */
+#if 0
 static void pktwr_ern_handler(struct qman_portal *qm, struct qman_fq *fq,
 				const struct qm_mr_entry *msg)
 {
 	return;
 }
+#endif
 
 /**
  \brief PktWire Error Frame Handler
@@ -134,7 +189,6 @@ static enum qman_cb_dqrr_result pktwr_sec_handler(struct qman_portal *p,
 						  const struct qm_dqrr_entry *dqrr)
 {
 	struct sg_entry_t *sg;
-	struct SecOutPd pOutPd;
 	ULONG addr;
 	ULONG status = 0;
 	
@@ -142,7 +196,7 @@ static enum qman_cb_dqrr_result pktwr_sec_handler(struct qman_portal *p,
 	addr = dqrr->fd.addr_lo;
 	status = dqrr->fd.status;
 	sg = (struct sg_entry_t *)__shmem_ptov(addr);
-    printf("\n fd status = 0x%x",status);
+    printf("\n fd status = 0x%x",(unsigned int)status);
 	//pktwr_dump("\nsec output buffer", (uint8_t *)(sg->addr_lo + sg->offset), 40);
 	pktwr_dump("\nsec output buffer", (uint8_t *)__shmem_ptov(sg->addr_lo) , 512/*sg->length*/);
     //printf("\nsg = 0x%x, sg->addr_lo = 0x%x, sg->offset = 0x%x ",sg,sg->addr_lo,sg->offset);
@@ -353,8 +407,8 @@ ULONG Sec_F8_Createfd_Single_Input(struct qm_fd *fd, struct SecF8InPd *pInPd)
 
 	
     pktwr_dump("sec job desc", (uint8_t *)descbuf, descbuf_size);
-	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov(input), pInPd->pInBuf[0]->dwDataSize);
-	pktwr_dump("sec input key", pInPd->pCipherKeyAddr, F8_KEY_LEN);
+	pktwr_dump("sec input data ", (uint8_t *)__shmem_ptov((unsigned long)input), (uint16_t)pInPd->pInBuf[0]->dwDataSize);
+	pktwr_dump("sec input key", (uint8_t *)pInPd->pCipherKeyAddr, (uint16_t)F8_KEY_LEN);
 
 	/* 保存需要传递的数据TODO */
 	//memcpy((pInPd->pOutBuf->pucBufStart + PKTWR_JD_OVERHEAD + PKTWR_SG_OVERHEAD), (UCHAR *)pInPd,sizeof(struct SecOutPd));
@@ -506,7 +560,7 @@ ULONG Sec_F8_Createfd_Multi_Input(struct qm_fd *fd, struct SecF8InPd *pInPd)
 
 	
     pktwr_dump("sec job desc", (uint8_t *)descbuf, descbuf_size);
-	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov(input), 16 * ucBufNum);
+	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov((unsigned long)input), 16 * ucBufNum);
 	pktwr_dump("sec input key", pInPd->pCipherKeyAddr, F8_KEY_LEN);
 
 	/* 保存需要传递的数据TODO */
@@ -666,7 +720,7 @@ ULONG Sec_F9_Createfd_Single_Input(struct qm_fd *fd, struct SecF9InPd *pInPd)
 
 	
     pktwr_dump("sec job desc", (uint8_t *)descbuf, descbuf_size);
-	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov(input), pInPd->pInBuf[0]->dwDataSize);
+	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov((unsigned long)input), pInPd->pInBuf[0]->dwDataSize);
 	pktwr_dump("sec input key", pInPd->pAuthKeyAddr, F9_KEY_LEN);
 
 	/* 保存需要传递的数据TODO */
@@ -851,7 +905,7 @@ ULONG Sec_F9_Createfd_Multi_Input(struct qm_fd *fd, struct SecF9InPd *pInPd)
 
 	
     pktwr_dump("sec job desc", (uint8_t *)descbuf, descbuf_size);
-	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov(input), 16 * ucBufNum);
+	pktwr_dump("sec input data", (uint8_t *)__shmem_ptov((unsigned long)input), 16 * ucBufNum);
 	pktwr_dump("sec input key", pInPd->pAuthKeyAddr, F9_KEY_LEN);
 
 	/* 保存需要传递的数据TODO */
@@ -1128,7 +1182,7 @@ ULONG Sec_F9F8_Createfd_Multi_Input(struct qm_fd *fd, struct SecF9F8InPd *pInPd)
 
 	
     pktwr_dump("sec job desc", (uint8_t *)descbuf, descbuf_size);
-	pktwr_dump("sec input data", &sg_out[3], 16 * ucBufNum);
+	pktwr_dump("sec input data", (uint8_t *)&sg_out[3], 16 * ucBufNum);
 	pktwr_dump("sec input key", pInPd->pCipherKeyAddr, F8_KEY_LEN);
 
 	/* 保存需要传递的数据TODO */
@@ -1159,7 +1213,7 @@ ULONG Sec_Enqueue(struct qm_fd *p_fd, ULONG ulfqid)
 	struct qman_fq *p_fq = NULL;/* TODO */
 	uint32_t ret = BSP_OK;
 	ULONG  qmportalnum = USDPAA_QMAN_PORTAL_NUM;
-	int intKey;
+	//int intKey;
 	
 #ifdef VOS_LINUX
 	qmportalnum = USDPAA_QMAN_PORTAL_NUM;
@@ -1222,7 +1276,6 @@ ULONG bsp_sec_jd_fq_init(void)
 {	
 	ULONG 					flags;
 	struct qm_mcc_initfq 		opts;
-	ULONG 						ulret = BSP_OK;
 	/* Initialize the reference to the Shared Descriptor. */
 
 	prehdr_desc = (struct preheader_s *)BspDpaShareMalloc(64, sizeof(struct preheader_s));
@@ -1484,7 +1537,6 @@ ULONG SecJdFqInit_ForCore0(void)
 {	
 	ULONG 					flags;
 	struct qm_mcc_initfq 		opts;
-	ULONG 						ulret = BSP_OK;
 	/* Initialize the reference to the Shared Descriptor. */
 
 	prehdr_desc = (struct preheader_s *)BspDpaShareMalloc(64, sizeof(struct preheader_s));
@@ -1676,13 +1728,13 @@ ULONG bsp_sec_init(void)
     ret = bsp_sec_reg_init();
 	if(BSP_ERROR == ret)
 	{
-    	printf("bsp_sec_reg_init Failed!%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, ret);
+    	printf("bsp_sec_reg_init Failed!%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, (int)ret);
 		return BSP_ERROR;
 	}
     ret = bsp_sec_jd_fq_init();
 	if(BSP_ERROR == ret)
 	{
-    	printf("bsp_sec_jd_fq_init Failed!,%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, ret);
+    	printf("bsp_sec_jd_fq_init Failed!,%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, (int)ret);
 		return BSP_ERROR;
 	}
 	return BSP_OK;
diff --git a/modules/usdpaa/src/secctrl.c b/modules/usdpaa/src/secctrl.c
index 0c77274..23af061 100644
--- a/modules/usdpaa/src/secctrl.c
+++ b/modules/usdpaa/src/secctrl.c
@@ -6,6 +6,7 @@
 
 /* Include the internal definitions of this module */
 #include "secctrl.h"
+#include <stdio.h>
 #define SEC_ADDR_BASE                         (unsigned long)0x300000
 #define SEC_JOBRING0_OFFSET                   (unsigned long)0x1000
 #define SEC_QI_OFFSEC                         (unsigned long)0x7000
@@ -20,10 +21,10 @@ t_SecQiMemMap*		p_QiMemMap = 0;
 void DumpSecReg(void)
 {
     printf("\n ************************* SEC HANDLE *********************************\n");
-	printf("\n  t_Sec = 0x%x \n", t_Sec);
-	printf("\n  p_SecMemMap = 0x%x \n", p_SecMemMap);
-	printf("\n  p_SecJq0MemMap = 0x%x \n", p_SecJq0MemMap);
-    printf("\n  p_QiMemMap = 0x%x \n", p_QiMemMap);
+	printf("\n  t_Sec = 0x%p \n", t_Sec);
+	printf("\n  p_SecMemMap = 0x%p \n", p_SecMemMap);
+	printf("\n  p_SecJq0MemMap = 0x%p \n", p_SecJq0MemMap);
+    printf("\n  p_QiMemMap = 0x%p \n", p_QiMemMap);
     
     printf("\n ************************* SEC GENERAL REGISTER *********************************\n");
     printf("  mcfgr = 0x%x \n",       BSP_GET_BIT32(p_SecMemMap->gen.mcfgr));
diff --git a/modules/ushell/src/bsp_ushell.c b/modules/ushell/src/bsp_ushell.c
index c7aa19c..c1d84a7 100644
--- a/modules/ushell/src/bsp_ushell.c
+++ b/modules/ushell/src/bsp_ushell.c
@@ -325,7 +325,7 @@ void bsp_ushell_init(UCHAR * ucpsymbname, UCHAR * ucpboardname)
             {
                 printf("\r    \r");
                 fflush(stdout);	
-                symFindByPartName(argstr[0]);
+                symFindByPartName((unsigned char *)argstr[0]);
                 printf("%s->",ucpboardname);
                 fflush(stdout);
                 add_history(msg); 
diff --git a/modules/ushell/src/bsp_ushell_elf.c b/modules/ushell/src/bsp_ushell_elf.c
index 821f23e..2a6d257 100644
--- a/modules/ushell/src/bsp_ushell_elf.c
+++ b/modules/ushell/src/bsp_ushell_elf.c
@@ -34,8 +34,9 @@
 /**************************** 私用头文件* **********************************/
 #include "bsp_ushell.h"
 /******************************* 局部宏定义 *********************************/
-
+#if 0
 #define S_IRUSR 00400
+#endif
 
 static ULONG symGetPosByValue(WORDPTR value);
 #define B2L(a, size) toLE((UCHAR*)&a, size)
diff --git a/modules/webserver/inc/wsIntrn.h b/modules/webserver/inc/wsIntrn.h
index 04a5f28..0c10b5f 100644
--- a/modules/webserver/inc/wsIntrn.h
+++ b/modules/webserver/inc/wsIntrn.h
@@ -48,7 +48,9 @@
 #include	<string.h>
 #include	<stdarg.h>
 
+#ifndef UEMF
 #define		 UEMF 1
+#endif
 #ifdef UEMF
        #undef UEMF
 	 #define UEMF
diff --git a/modules/webserver/src/balloc.c b/modules/webserver/src/balloc.c
index b22aa3c..f661520 100644
--- a/modules/webserver/src/balloc.c
+++ b/modules/webserver/src/balloc.c
@@ -401,6 +401,7 @@ void bfree(B_ARGS_DEC, void *mp)
 #ifdef B_VERIFY_CAUSES_SEVERE_OVERHEAD
 	bFillBlock(bp, memSize);
 #endif
+	memSize = memSize;
 
 /*
  *	Simply link onto the head of the relevant q
diff --git a/modules/webserver/src/base64.c b/modules/webserver/src/base64.c
index 0e400d1..938c548 100644
--- a/modules/webserver/src/base64.c
+++ b/modules/webserver/src/base64.c
@@ -134,7 +134,7 @@ void websEncode64(char_t *outbuf, char_t *string, int outlen)
 		shift = 18;
 		for (i = ++j; i < 4 && op < &outbuf[outlen] ; i++) {
 			x = (shiftbuf >> shift) & 0x3f;
-			*op++ = alphabet64[(shiftbuf >> shift) & 0x3f];
+			*op++ = alphabet64[x];
 			shift -= 6;
 		}
 /*
diff --git a/modules/webserver/src/ejlex.c b/modules/webserver/src/ejlex.c
index c129daf..ec161b2 100644
--- a/modules/webserver/src/ejlex.c
+++ b/modules/webserver/src/ejlex.c
@@ -222,6 +222,7 @@ static int getLexicalToken(ej_t* ep, int state)
 	a_assert(ip);
 
 	inq = &ip->script;
+	inq = inq;
 	tokq = &ip->tokbuf;
 
 	ep->tid = -1;
diff --git a/modules/webserver/src/emfdb.c b/modules/webserver/src/emfdb.c
index 1cbc81e..d614aef 100644
--- a/modules/webserver/src/emfdb.c
+++ b/modules/webserver/src/emfdb.c
@@ -237,7 +237,7 @@ void dbZero(int did)
 int dbSearchStr(int did, char_t *tablename, 
 	char_t *colName, char_t *value, int flags)
 {
-	int			tid, nRows, nColumns, column;
+	int			tid, nRows,  column;
    int match = 0;
 	dbTable_t	*pTable;
 
@@ -254,7 +254,6 @@ int dbSearchStr(int did, char_t *tablename,
 		return DB_ERR_TABLE_NOT_FOUND;
 	}
 	
-	nColumns = pTable->nColumns;
 	nRows = pTable->nRows;
 	column = GetColumnIndex(tid, colName);
 	a_assert (column >= 0);
@@ -399,7 +398,7 @@ int dbDeleteRow(int did, char_t *tablename, int row)
 
 int dbSetTableNrow(int did, char_t *tablename, int nNewRows)
 {
-	int			nRet, tid, nRows, nColumns;
+	int			nRet, tid, nRows;
 	dbTable_t	*pTable;
 
 	a_assert(tablename);
@@ -416,7 +415,6 @@ int dbSetTableNrow(int did, char_t *tablename, int nNewRows)
 
 	a_assert(pTable);
 	if (pTable) {
-		nColumns = pTable->nColumns;
 		nRows = pTable->nRows;
 		nRet = 0;
 
diff --git a/modules/webserver/src/misc.c b/modules/webserver/src/misc.c
index 84661f2..213de36 100644
--- a/modules/webserver/src/misc.c
+++ b/modules/webserver/src/misc.c
@@ -424,7 +424,7 @@ static int dsnprintf(char_t **s, int size, char_t *fmt, va_list arg, int msize)
 /*
  *	Return the length of a string limited by a given length
  */
-
+#if 0
 static int strnlen_1(char_t *s, unsigned int n)
 {
 	unsigned int 	len;
@@ -432,6 +432,7 @@ static int strnlen_1(char_t *s, unsigned int n)
 	len = gstrlen(s);
 	return min(len, n);
 }
+#endif
 
 /******************************************************************************/
 /*
diff --git a/modules/webserver/src/security.c b/modules/webserver/src/security.c
index fd8dd7b..6e43de0 100644
--- a/modules/webserver/src/security.c
+++ b/modules/webserver/src/security.c
@@ -104,6 +104,7 @@ int websSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
  *	Get the critical request details
  */
 	type = websGetRequestType(wp);
+	type = type;
 	password = websGetRequestPassword(wp);
 	userid = websGetRequestUserName(wp);
 	flags = websGetRequestFlags(wp);
diff --git a/modules/webserver/src/sockGen.c b/modules/webserver/src/sockGen.c
index 6d99833..4178647 100644
--- a/modules/webserver/src/sockGen.c
+++ b/modules/webserver/src/sockGen.c
@@ -423,7 +423,7 @@ static void socketAccept(socket_t *sp)
  *	Accept the connection and prevent inheriting by children (F_SETFD)
  */
 	len = sizeof(struct sockaddr_in);
-	if ((newSock = accept(sp->sock, (struct sockaddr *) &addr, (int *) &len)) < 0) {
+	if ((newSock = accept(sp->sock, (struct sockaddr *) &addr, (socklen_t *) &len)) < 0) {
 		return;
 	}
 #ifndef __NO_FCNTL
@@ -511,7 +511,7 @@ int socketGetInput(int sid, char *buf, int toRead, int *errCode)
 	if (sp->flags & SOCKET_DATAGRAM) {
 		len = sizeof(server);
 		bytesRead = recvfrom(sp->sock, buf, toRead, 0,
-			(struct sockaddr *) &server, &len);
+			(struct sockaddr *) &server, (socklen_t *)&len);
 	} else {
 		bytesRead = recv(sp->sock, buf, toRead, 0);
 	}
@@ -1092,6 +1092,7 @@ int socketSetBlock(int sid, int on)
 	int				oldBlock;
 
 	flag = iflag = !on;
+	flag = flag;
 
 	if ((sp = socketPtr(sid)) == NULL) {
 		a_assert(0);
diff --git a/modules/webserver/src/umui.c b/modules/webserver/src/umui.c
index f447d79..c9a5c5a 100644
--- a/modules/webserver/src/umui.c
+++ b/modules/webserver/src/umui.c
@@ -230,13 +230,12 @@ static void formDisplayUser(webs_t wp, char_t *path, char_t *query)
 static int aspGenerateUserList(int eid, webs_t wp, int argc, char_t **argv)
 {
 	char_t	*userid;
-	int		row, nBytesSent, nBytes;
+	int nBytesSent, nBytes;
 
 	a_assert(wp);
 
 	nBytes = websWrite(wp, 
 		T("<SELECT NAME=\"user\" SIZE=\"3\" TITLE=\"Select a User\">"));
-	row = 0;
 	userid = umGetFirstUser();
 	nBytesSent = 0;
 
@@ -373,11 +372,10 @@ static void formDeleteGroup(webs_t wp, char_t *path, char_t *query)
 static int aspGenerateGroupList(int eid, webs_t wp, int argc, char_t **argv)
 {
 	char_t	*group;
-	int		row, nBytesSent, nBytes;
+	int		 nBytesSent, nBytes;
 
 	a_assert(wp);
 
-	row = 0;
 	nBytesSent = 0;
 	nBytes = websWrite(wp, 
 		T("<SELECT NAME=\"group\" SIZE=\"3\" TITLE=\"Select a Group\">"));
@@ -496,11 +494,11 @@ static int aspGenerateAccessLimitList(int eid, webs_t wp,
 									  int argc, char_t **argv)
 {
 	char_t	*url;
-	int		row, nBytesSent, nBytes;
+	int		 nBytesSent, nBytes;
 
 	a_assert(wp);
 
-	row = nBytesSent = 0;
+	nBytesSent = 0;
 	url = umGetFirstAccessLimit();
 	nBytes = websWrite(wp, 
 		T("<SELECT NAME=\"url\" SIZE=\"3\" TITLE=\"Select a URL\">"));
diff --git a/modules/webserver/src/webcomp.c b/modules/webserver/src/webcomp.c
index c9a31e6..d9775da 100644
--- a/modules/webserver/src/webcomp.c
+++ b/modules/webserver/src/webcomp.c
@@ -128,7 +128,7 @@ static int compile(char_t *fileList, char_t *prefix)
  */
 	nFile = 0;
 	while (fgets(file, sizeof(file), lp) != NULL) {
-		if ((p = strchr(file, '\n')) || (p = strchr(file,'\r'))) {
+		if ((p = (unsigned char *)strchr((char *)file, '\n')) || (p = (unsigned char *)strchr((char *)file,'\r'))) {
 			*p = '\0';
 		}
 		if (*file == '\0') {
@@ -147,7 +147,7 @@ static int compile(char_t *fileList, char_t *prefix)
 			p = (unsigned char *)buf;
 			for (i = 0; i < len; ) {
 				fprintf(stdout, "    ");
-				for (j = 0; p < &buf[len] && j < 16; j++, p++) {
+				for (j = 0; p < (unsigned char *)&buf[len] && j < 16; j++, p++) {
 					fprintf(stdout, "%3d,", *p);
 				}
 				i += j;
@@ -172,7 +172,7 @@ static int compile(char_t *fileList, char_t *prefix)
 	}
 	nFile = 0;
 	while (fgets(file, sizeof(file), lp) != NULL) {
-		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
+		if ((p = (unsigned char *)strchr((char *)file, '\n')) || (p = (unsigned char *)strchr((char *)file, '\r'))) {
 			*p = '\0';
 		}
 		if (*file == '\0') {
@@ -198,7 +198,7 @@ static int compile(char_t *fileList, char_t *prefix)
 			continue;
 		}
 		fprintf(stdout, "    { T(\"/%s\"), page_%d, %d },\n", cp, nFile, 
-			sbuf.st_size);
+			(int)sbuf.st_size);
 		nFile++;
 	}
 	fclose(lp); 
diff --git a/modules/webserver/src/webmain.c b/modules/webserver/src/webmain.c
index a0d35b4..2f50a79 100644
--- a/modules/webserver/src/webmain.c
+++ b/modules/webserver/src/webmain.c
@@ -50,8 +50,8 @@ static int			finished;						/* Finished flag */
 /****************************** Forward Declarations **************************/
 
 static int 	initWebs();
-static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
-static void formTest(webs_t wp, char_t *path, char_t *query);
+//static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
+//static void formTest(webs_t wp, char_t *path, char_t *query);
 static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
 				int arg, char_t *url, char_t *path, char_t *query);
 extern void defaultErrorHandler(int etype, char_t *msg);
@@ -62,6 +62,8 @@ static void memLeaks();
 #endif
 extern int web_app();
 extern int ps2WebsSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path, char_t *query);
+extern unsigned long BspGetCtrlNetIp(unsigned char* pucAddr);
+extern void BspWebsInit(void );
 /*********************************** Code *************************************/
 /*
  *	Main -- entry point from LINUX
@@ -164,7 +166,7 @@ static int initWebs()
 		error(E_L, E_LOG, T("Can't get hostname"));
 		return -1;
 	}
-	BspGetCtrlNetIp(host);
+	BspGetCtrlNetIp((unsigned char *)host);
 	printf("web server's ip addr ->%s\n",host);
 	if ((hp = gethostbyname(host)) == NULL) 
 	{
@@ -255,6 +257,7 @@ static int initWebs()
  *	"localhost/asp.asp" to test.
  */
 
+#if 0
 static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
 {
 	char_t	*name, *address;
@@ -265,6 +268,7 @@ static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
 	}
 	return websWrite(wp, T("Name: %s, Address %s"), name, address);
 }
+
 
 /******************************************************************************/
 /*
@@ -284,7 +288,7 @@ static void formTest(webs_t wp, char_t *path, char_t *query)
 	websFooter(wp);
 	websDone(wp, 200);
 }
-
+#endif
 /******************************************************************************/
 /*
  *	Home page handler
@@ -503,6 +507,7 @@ int uservalid(int eid, webs_t wp, int argc, char_t **argv)
 * 返回值: 0 正常，其他错误
 * 说明:
 ******************************************************************************/
+#if 0
 static unsigned char isAccessPagePub(unsigned char *url)
 {
 	if (NULL == gstrstr((const char *)url, T("function.asp")) && (0 != gstrcmp((const char *)url, T("/goform/loginSet")))
@@ -513,6 +518,7 @@ static unsigned char isAccessPagePub(unsigned char *url)
 	}		
 	return 1;
 }
+#endif
 
 #define NO_PARAM				(-10)
 #define NO_NEXT					(-20)	
@@ -680,7 +686,7 @@ int GetQueryString(webs_t wp, char *query, T_QueryStruct *pWebValue)
 ******************************************************************************/
 void loginSet(webs_t wp, char_t *path, char_t *query)
 {
-	T_QueryStruct	QueryTab[VALUEMAX] = {0};
+	T_QueryStruct	QueryTab[VALUEMAX];
     GetQueryString(wp, query, QueryTab);
 	printf("QueryTab[0].VarStr->%s\r\n",QueryTab[0].VarStr);
 	printf("QueryTab[1].VarStr->%s\r\n",QueryTab[1].VarStr);
@@ -712,7 +718,7 @@ void loginSet(webs_t wp, char_t *path, char_t *query)
 
 void BspFactoryParmConfig(webs_t wp, char_t *path, char_t *query)
 {
-	T_QueryStruct	QueryTab[VALUEMAX] = {0};
+	T_QueryStruct	QueryTab[VALUEMAX];
     GetQueryString(wp, query, QueryTab);
 	printf("BspFactoryParmConfig QueryTab[0].VarStr->%s\r\n",QueryTab[0].VarStr);
 	printf("BspFactoryParmConfig QueryTab[1].VarStr->%s\r\n",QueryTab[1].VarStr);
@@ -747,7 +753,7 @@ void BspFactoryParmConfig(webs_t wp, char_t *path, char_t *query)
 
 void BspNetParmConfig(webs_t wp, char_t *path, char_t *query)
 {
-	T_QueryStruct	QueryTab[VALUEMAX] = {0};
+	T_QueryStruct	QueryTab[VALUEMAX];
     GetQueryString(wp, query, QueryTab);
 	printf("BspNetParmConfig QueryTab[0].VarStr->%s\r\n",QueryTab[0].VarStr);
 	printf("BspNetParmConfig QueryTab[1].VarStr->%s\r\n",QueryTab[1].VarStr);
@@ -777,7 +783,7 @@ void BspNetParmConfig(webs_t wp, char_t *path, char_t *query)
 
 void BspStationParmConfig(webs_t wp, char_t *path, char_t *query)
 {
-	T_QueryStruct	QueryTab[VALUEMAX] = {0};
+	T_QueryStruct	QueryTab[VALUEMAX];
     GetQueryString(wp, query, QueryTab);
 	printf("BspStationParmConfig QueryTab[0].VarStr->%s\r\n",QueryTab[0].VarStr);
 	printf("BspStationParmConfig QueryTab[1].VarStr->%s\r\n",QueryTab[1].VarStr);
@@ -860,6 +866,7 @@ int Get_ProductParmConfig(int eid, webs_t wp, int argc, char_t **argv)
     {
 	    ejSetResult(eid,"N/A");
 	}
+    return 0;
 }
 
 int Get_BaseStationNetParmConfig(int eid, webs_t wp, int argc, char_t **argv)
@@ -896,6 +903,8 @@ int Get_BaseStationNetParmConfig(int eid, webs_t wp, int argc, char_t **argv)
     {
 	    ejSetResult(eid,"N/A");
 	}
+
+    return 0;
 }
 
 int Get_BaseStationConfig(int eid, webs_t wp, int argc, char_t **argv)
@@ -956,6 +965,7 @@ int Get_BaseStationConfig(int eid, webs_t wp, int argc, char_t **argv)
     {
 	    ejSetResult(eid,"N/A");
 	}
+    return 0;
 }
 
 
diff --git a/modules/webserver/src/webs.c b/modules/webserver/src/webs.c
index 57af3f2..1f552c0 100644
--- a/modules/webserver/src/webs.c
+++ b/modules/webserver/src/webs.c
@@ -827,7 +827,7 @@ static int websParseFirst(webs_t wp, char_t *text)
 
 static void websParseRequest(webs_t wp)
 {
-	char_t	*authType, *upperKey, *cp, *browser, *lp, *key, *value;
+	char_t	*authType, *upperKey, *cp,  *lp, *key, *value;
 
 	a_assert(websValid(wp));
 
@@ -841,7 +841,6 @@ static void websParseRequest(webs_t wp)
  *	We rewrite the header as we go for non-local requests.  NOTE: this
  * 	modifies the header string directly and tokenizes each line with '\0'.
  */
-	browser = NULL;
 	for (lp = (char_t*) wp->header.servp; lp && *lp; ) {
 		cp = lp;
 		if ((lp = gstrchr(lp, '\n')) != NULL) {
