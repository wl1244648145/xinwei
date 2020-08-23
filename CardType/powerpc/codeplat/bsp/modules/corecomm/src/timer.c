#include "../inc/timer.h"
extern T_MPIC_VECS *pMpic_vecs;
extern ULONG g_ulTestMsg[8];
extern ULONG g_ulTestIpi[4];
extern ULONG gulMpicRegBar;

       
UINT32	g_ulTimerClkFreq = (600000000/2)>> 3;


/* 定义回调函数指针 */
typedef struct 
{
	void (*pgTimerCallBack)(unsigned long ulCallBackParam); 

}GLOBAL_TIMER_CALLBACK_FUNC;

GLOBAL_TIMER_CALLBACK_FUNC pTimer_Fun[8];

ULONG g_ulCallBackParam[8] = {0}; 
extern ULONG g_ulCurCpuId;

ULONG g_ulTestMpicTimer = 0;

extern UINT32 BspGetCurCpu(void );
extern VOID BspIntcEnable(const ULONG ulIntNo);
extern char * readline (char * cmd);
/************************************************************/
/*   内部函数声明                                           */
/************************************************************/

void BspTimerCBIsr(UINT32 udTimerIndex);
UINT32 BspTimerCBInstall(UINT32 udTimerIndex, T_TimerCBParam *ptCBParam);
int BspMpicTimerConfig(ULONG ulTimerIndex,T_MPIC_TIMER_CONFIG * ptTimerCfg );
int BspTimerSetBaseCount(UINT32 udTimerIndex,ULONG ulCountVal);
int BspSetTimerFrq(UINT32 udTimerIndex,ULONG ulFrqVal);
void BspTimerCallBack(VOIDFUNCPTR pCallBack);

/************************************************************/
/*   外部函数声明                                           */
/************************************************************/
extern	void  BspTestExtIRQAndInstall(void);
//extern void   BspTimerCallBack(UCHAR ucFlag);
extern VOID   BspIntcDisable (const UINT32 ulIntNo);
extern UINT32 BspIntcHookRegister(T_IntcAttrs * const ptIntcAttrs);

void BspTimerCallBack(VOIDFUNCPTR pCallBack)
{	
    if (NULL != pCallBack)
    {
	    BspSetTimerInit(1,1,pCallBack);
    }
}

void BspTimerToNrl2CallBack(VOIDFUNCPTR pCallBack)
{
    if (NULL != pCallBack)
    {
          BspSetTimerInit(2,1,pCallBack);
    }  
}

UINT32 BspTimerInit(UINT32 udTimerIndex,T_TimerInitParam *ptTmrInitParam)
{
	T_IntcAttrs tIntcAttrs;
	T_MPIC_TIMER_CONFIG tMpicTimerConfig;
	ULONG ulBaseCnt = 0;
	ULONG ulFrq = 0;
    ULONG ulTimerVector =0;
	
    if(NULL == ptTmrInitParam)
    {
        printf("BspTimerInit Input Para ptTmrInitParam = NULL!\n");
		return BSP_E_POINTER_NULL;
    }
	if(udTimerIndex > 7)
	{
	    printf("Input Para timer Index = [%d] error!\n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}

	if(udTimerIndex < 4)
	{
	    ulTimerVector = MPIC_VEC_GTA_IRQ0 + udTimerIndex;
	}
	else
	{
	    ulTimerVector = MPIC_VEC_GTB_IRQ0 + udTimerIndex-4;
	}
  
	if(NULL == ptTmrInitParam->pTimerCB)
	{   
	    BspIntcDisable(ulTimerVector);
		printf("Timer Call Back Func is NULL !!\n");
	    return BSP_E_POINTER_NULL;
	}
	else
	{
		pTimer_Fun[udTimerIndex].pgTimerCallBack = ptTmrInitParam->pTimerCB;
	    g_ulCallBackParam[udTimerIndex] = ptTmrInitParam->udIntParam;
	}
	ulFrq = g_ulTimerClkFreq;
	if(BSP_OK != BspSetTimerFrq(udTimerIndex,ulFrq))
	{
	    printf("Timer[%d] Set Frq failed!!!\n",udTimerIndex);
		return BSP_ERROR;
	}
	ulBaseCnt = (ptTmrInitParam->udClkFreq) * (ptTmrInitParam->udPrd);
	if(BSP_OK != BspTimerSetBaseCount(udTimerIndex,ulBaseCnt))
	{
	    printf("Timer Set Base Count failed!!!\n");
		return BSP_ERROR;
	}
	tMpicTimerConfig.ucCascadeTime = 0;
	tMpicTimerConfig.ucClockRatio  = 0;
	tMpicTimerConfig.ucRollOver    = 0;
	tMpicTimerConfig.ucRealTimeMod = ptTmrInitParam->udClkSrc;
	
	if(BSP_OK != BspMpicTimerConfig(udTimerIndex,(T_MPIC_TIMER_CONFIG *)&tMpicTimerConfig))
	{
	    printf("Timer Config failed!!!\n");
		return BSP_ERROR;
	}
	memset((UCHAR *)&tIntcAttrs,0,sizeof(T_IntcAttrs));

	tIntcAttrs.IsrFunc       = (VOIDFUNCPTR)BspTimerCBIsr;
	tIntcAttrs.udarg         = udTimerIndex;
	tIntcAttrs.udPrio        = 10;
	tIntcAttrs.ucTrgMode     = 0x11;
    tIntcAttrs.udIntNo   = ulTimerVector;

	BspIntcHookRegister((T_IntcAttrs *)&tIntcAttrs);	
	return OK;
	
}
 

ULONG BspTimerIntDstCoreSet(UINT32 udTimerIndex,ULONG ulDstCpu)
{
    if(ulDstCpu > 7)
    {
		return BSP_E_INPUTPARA_ERROR;
    }
    if(udTimerIndex < 4)
    {
        MPIC_REG_WRITE(MPIC_TMA0_DES_REG+0x40*udTimerIndex,1<<ulDstCpu);
		
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
        MPIC_REG_WRITE(MPIC_TMB0_DES_REG+0x40*(udTimerIndex-4),1<<ulDstCpu);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return OK;
    
}

 

UINT32 BspTimerStart(UINT32 udTimerIndex)
{
    ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		ulRegValTemp &= ~P3_MPIC_GT_DISABLE;
		MPIC_REG_WRITE(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
		ulRegValTemp &= ~P3_MPIC_GT_DISABLE;
		MPIC_REG_WRITE(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return OK;

}

 

UINT32 BspTimerStop(UINT32 udTimerIndex)
{
    ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		ulRegValTemp |= P3_MPIC_GT_DISABLE;
		MPIC_REG_WRITE(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
		ulRegValTemp |= P3_MPIC_GT_DISABLE;
		MPIC_REG_WRITE(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return OK;

}


 

UINT32 BspTimerGetBaseCountReg(UINT32 udTimerIndex)
{
    ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return ulRegValTemp;

}


 

UINT32 BspTimerClearCount(UINT32 udTimerIndex)
{
    ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		ulRegValTemp = ulRegValTemp & P3_MPIC_GT_DISABLE;
		MPIC_REG_WRITE(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
		ulRegValTemp = ulRegValTemp & P3_MPIC_GT_DISABLE;
		MPIC_REG_WRITE(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return OK;
}

 

UINT32 BspTimerGetBaseCount(UINT32 udTimerIndex)
{
    ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		ulRegValTemp &= ~P3_MPIC_GT_DISABLE;
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
		ulRegValTemp &= ~P3_MPIC_GT_DISABLE;
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return ulRegValTemp;

}


 

int BspTimerSetBaseCount(UINT32 udTimerIndex,ULONG ulCountVal)
{
    ULONG ulRegValTemp = 0;

	if(ulCountVal > P3_MPIC_GT_BASECNT_MAX_VAL)
	{
		return BSP_E_INPUTPARA_ERROR;
	}
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		ulRegValTemp = (ulCountVal & ~P3_MPIC_GT_DISABLE) |(ulRegValTemp & P3_MPIC_GT_DISABLE);
		MPIC_REG_WRITE(MPIC_TMA0_BASE_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
		ulRegValTemp = (ulCountVal & ~P3_MPIC_GT_DISABLE) |(ulRegValTemp & P3_MPIC_GT_DISABLE);
		MPIC_REG_WRITE(MPIC_TMB0_BASE_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return BSP_OK;
    
}


UINT32 BspTimerGetCurCount(UINT32 udTimerIndex)
{
    ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_CUR_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
		ulRegValTemp &= P3_MPIC_GT_CURCNT_VAL_MASK;
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_CUR_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
		ulRegValTemp &= P3_MPIC_GT_CURCNT_VAL_MASK;
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return ulRegValTemp;

}

 

UINT32 BspTimerCBInstall(UINT32 udTimerIndex, T_TimerCBParam *ptCBParam)
{
	if(NULL == ptCBParam)
	{
	    printf("InPut Para error! ptCBParam = NULL!\n");
		return BSP_E_POINTER_NULL;
	}
	if((udTimerIndex > 7)||(udTimerIndex < 0))
	{
	    printf("InPut Para  udTimerIndex = [%d] error!\n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	pTimer_Fun[udTimerIndex].pgTimerCallBack = ptCBParam->pfTimerCB;
	g_ulCallBackParam[udTimerIndex] = ptCBParam->udIntParam;	
	return OK;

}

UINT32 BspTimerCBunInstall(UINT32 udTimerIndex, T_TimerCBParam *ptCBParam)
{
 
	if(NULL == ptCBParam)
	{
	    printf("InPut Para error! ptCBParam = NULL!\n");
		return BSP_E_POINTER_NULL;
	}
	if((udTimerIndex > 7)||(udTimerIndex < 0))
	{
	    printf("InPut Para  udTimerIndex = [%d] error!\n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}

	while(ptCBParam->pfTimerCB!= NULL)
	{
	    pTimer_Fun[udTimerIndex].pgTimerCallBack = NULL;
	    g_ulCallBackParam[udTimerIndex] = 0;
	}
	return OK;

}

 

void BspTimerCBIsr(UINT32 udTimerIndex)
{	
    if(NULL != pTimer_Fun[udTimerIndex].pgTimerCallBack)
    {    
        (*(pTimer_Fun[udTimerIndex].pgTimerCallBack))(g_ulCallBackParam[udTimerIndex]);
    }
    else
    {
        printf("BspTimerCBIsr for timer%d not installed!\n",udTimerIndex);
    }
}


ULONG BspGetMpicTBCurCnt(UINT32 udTimerIndex)
{
	ULONG ulRegValTemp = 0;
	
    if(udTimerIndex < 4)
    {
        MPIC_REG_READ(MPIC_TMA0_CUR_COUNT_REG + 0x40*udTimerIndex,ulRegValTemp);
    }
	else if((udTimerIndex < 8)&&(udTimerIndex > 3))
	{
	    MPIC_REG_READ(MPIC_TMB0_CUR_COUNT_REG + 0x40*(udTimerIndex-4),ulRegValTemp);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return ulRegValTemp;
}


int BspSetTimerFrq(UINT32 udTimerIndex,ULONG ulFrqVal)
{
    if(udTimerIndex < 4)
	{
	    MPIC_REG_WRITE(MPIC_TMA_FREQ_REG ,ulFrqVal);
		
	}
	else if((udTimerIndex > 3)&&(udTimerIndex < 8))
	{
	    MPIC_REG_WRITE(MPIC_TMB_FREQ_REG ,ulFrqVal);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return OK;
}

 
ULONG BspReadTimerFrq(UINT32 udTimerIndex)
{
    ULONG ulTimerFrq = 0;
    if(udTimerIndex < 4)
	{
	    MPIC_REG_READ(MPIC_TMA_FREQ_REG,ulTimerFrq);
		
	}
	else if((udTimerIndex > 3)&&(udTimerIndex < 8))
	{
	    MPIC_REG_READ(MPIC_TMB_FREQ_REG,ulTimerFrq);
	}
	else
	{
	    printf("Input Para udTimerIndex = [%d] error \n",udTimerIndex);
		return BSP_E_INPUTPARA_ERROR;
	}
	return ulTimerFrq;

}
 
int BspMpicTimerConfig(ULONG ulTimerIndex,T_MPIC_TIMER_CONFIG * ptTimerCfg )
{
    ULONG ulTemp = 0;
	if(NULL == ptTimerCfg)
	{
	    return BSP_E_POINTER_NULL;
	}
	
	ulTemp = (ptTimerCfg->ucCascadeTime & 0x7)|((ptTimerCfg->ucClockRatio & 0x3) << 8)|
		     ((ptTimerCfg->ucRealTimeMod & 0x1)<< 16)|((ptTimerCfg->ucRollOver & 0x7)<< 24);
	
	if(ulTimerIndex < 4)
	{
	    MPIC_REG_WRITE(MPIC_TMA_CTRL ,ulTemp);
		
	}
	else if((ulTimerIndex > 3)&&(ulTimerIndex < 8))
	{
	    MPIC_REG_WRITE(MPIC_TMB_CTRL ,ulTemp);
	}
	else
	{
		return BSP_E_INPUTPARA_ERROR;
	}
	return BSP_OK;
    
}

ULONG BspMpicTimeBaseInit(void)
{
    T_MPIC_TIMER_CONFIG tMpicTimerConfig;
	ULONG ulFrq = 0;
    tMpicTimerConfig.ucCascadeTime = 0;
    tMpicTimerConfig.ucClockRatio  = 0;
	tMpicTimerConfig.ucRealTimeMod = 0;
	tMpicTimerConfig.ucRollOver    = 0;
	BspTimerSetBaseCount(TIME_BASE_TIMER_INDEX,0x7fffffff);
	ulFrq = g_ulTimerClkFreq;
	if(BSP_OK != BspSetTimerFrq(TIME_BASE_TIMER_INDEX,ulFrq))
	{
	    printf("Timer[%d] Set Frq failed!!!\n",TIME_BASE_TIMER_INDEX);
		return BSP_ERROR;
	}
	/* 计数器启动 */
	if(BSP_OK != BspTimerStart(TIME_BASE_TIMER_INDEX))
	{
	    printf("Timer[%d] Start failed!!!\n",TIME_BASE_TIMER_INDEX);
		return BSP_ERROR;
	}
	return BSP_OK;
}

void BspSetTimerInit(ULONG ulTimerIndex,ULONG ulTimeVal,VOIDFUNCPTR pCallBack)
{
    T_TimerInitParam tTimeInitPara;
	ULONG ulTemp = 0;
	ULONG ulCpu = 0;
	
	printf("定时器号 : [%d] Time:[%d]ms\n",ulTimerIndex,ulTimeVal);
	
	tTimeInitPara.udPrd = ulTimeVal;
	tTimeInitPara.udClkFreq = g_ulTimerClkFreq/1000;/*ms级*/
	tTimeInitPara.udClkSrc = 0;
	tTimeInitPara.pTimerCB = (VOIDFUNCPTR)pCallBack;
	if(0 == ulTimerIndex)
	{
	    tTimeInitPara.udIntParam = 0;/*ipi*/
	}
	else if(1 == ulTimerIndex)
	{
	    tTimeInitPara.udIntParam = 1;/*msg*/
	}
    else if( 2 == ulTimerIndex)
    {
        tTimeInitPara.udIntParam = 2;
    }
	
	BspTimerInit(ulTimerIndex,(T_TimerInitParam*)&tTimeInitPara);

	ulCpu = BspGetCurCpu();
    
	BspTimerIntDstCoreSet(ulTimerIndex,ulCpu);
	BspTimerStart(ulTimerIndex);
}




























































