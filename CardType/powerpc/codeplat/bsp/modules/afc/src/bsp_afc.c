/*******************************************************************************
* *  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 文件名称:  bsp_afc.c
* 功    能:
* 版    本:  V0.1
* 编写日期:
* 说    明:  无
* 修改历史:
* 修改日期           修改人  BugID/CRID     修改内容
*------------------------------------------------------------------------------
*                                                     创建文件
*******************************************************************************/
/******************************* 包含文件声明 *********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/inotify.h>
/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "bsp_epld_ext.h"
#include "bsp_gps_ext.h"
#include "bsp_afc.h"
#include "../inc/bsp_afc_matrix.h"
#include "../inc/bsp_afc_phase.h"
#include "bsp_intpro_ext.h"
/******************************* 局部宏定义 ***********************************/
/******************************* 全局变量定义/初始化 **************************/
sem_t  g_semb_getphas_sendac;
sem_t g_semb_afc_main_proc;
int g_gps_lock=0;
pthread_mutex_t  g_mp_spi_dd_phase_epld = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_mp_afc_protect = PTHREAD_MUTEX_INITIALIZER;

pthread_t   t_afc_get_freq_phase_thread;
pthread_t   t_afc_func_thread;

u32 g_u32Afcprint_level;
u32 g_u32AfcDacCnt1=0;
u32 g_u32AfcDacCnt2=0;
u32 g_u32AfcPrintCnt=0;
u8  g_u8ResetCause=0;

STRU_AFC_COUNTER g_struAfcCounter;
STRU_AFC_DAC g_struAfcDac;
STRU_AFC_STATE g_struAfcStat;
STRU_AFC_PHASE g_struAfcPhase;
STRU_AFC_FREQUENCE g_struAfcFrq;
STRU_AFC_COMP  g_struAfcComp;
STRU_RESET_CAUSE g_struResetReason;
STRU_AFC_DAC_SAVE g_struDACSave;
u16 g_u16AfcGpsStatIndex=0;
u32 g_u32PhaseDifErrCnt=0;
u16 g_u16AheadData=0;
u16 g_u16LagData=0;

s32 s32Value20scur = 0;
s32 s32Value20slast = 0;

u32 u32Value20sfirstcnt = 0;
u32 u32Cnt20s_head=0;
u32 u32Cnt20s_tail=0;;
s32 s32Value20stotal[CNT_WINDOWS];
u32 u32Value20shstcnt = 0;

float u32OCCoefbak = 0 ;
u32 u32DacValuebak = 0;

u32 g_u32afc_print = 0;
u32 g_u32afc_20sfrqcnt_start = 0;

s32 CntDif1s_max =0;
s32 CntDif1s_min =0;
/******************************* 局部函数原型声明 *****************************/
s32 bsp_afc_init(void);
double afc_freqdif_avg(const vu32 *pu32Array, u8 u8CurHis);
double  afc_round(double data);
s32 afc_check_ocxo(void);
s32 afc_freq_modulate(void);
s32 afc_phase_modulate(void);
s32  afc_check_locked(void);
s32  afc_state_change(u8 u8changeto);
void  afc_cpld_clk_rst_oc(void);
void afc_get_OCcoef(u16 u16lowDAC, u16 u16higDAC, float *padjCoef);
void  afc_comp_coef_cal(u32 u32datalen);
void afc_comp_coef_adjust(u32 u32datalen);
void  afc_comp_func(void);
s32 t_afc_get_freq_phase(void);
s32 t_afc_func(void);
void afc_dac_save(char  *dacsave);
static void afc_gps_led_control();

char AFC_DAC_PATH[] = "/mnt/csi/dac.txt";
char AFC_LOG_PATH[] = "/mnt/csi/afc.log";

s32 g_afc_ocxo_flag = 0;
s8 alarm_syn_change[100] =  {0};
s8 alarm_afc_status_change[100] =  {0};
s8 alarm_afc_ocxo[100] = {0};
s8 alarm_afc_timeout[100] =  "AFC holdover timeout alarm";
s8 alarm_afc_abnormal[100] = "AFC abnormal alarm";
u32 u32en20s = 0;
u32 s32Value20s_end = 0;
extern s32 s32DspDownldStatus;

extern Clock_Set_Tc_Type g_Clock_Set_AllDate;
extern u8 g_ublox_gps_type;
extern u8 g_GNSS_flag;
s8 afc_log[AFC_LOG_LENTH][200]= {0};

u16 bts_synsrc_type = SYNSRC_LOCAL;
//extern void  interpro_syn_status_change_notify(u16 result);
/******************************* 函数实现 *************************************/
/*******************************************************************************
* 函数名称: bsp_afc_cpld_mutex_init
* 功    能:
* 相关文档:
* 函数类型: s32
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态值，成功(AFC_OK)或失败
* 说   明:
*******************************************************************************/
s32 bsp_afc_cpld_mutex_init(void)
{
    s32 s32ret = 0;
    s32ret =pthread_mutex_init(&g_mp_spi_dd_phase_epld, NULL);
    return s32ret;
}

/*******************************************************************************
* 函数名称: bsp_afc_protect_mutex_init
* 功    能: afc模块初始化函数
* 相关文档:
* 函数类型: s32
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态值，成功(AFC_OK)或失败
* 说   明:
*******************************************************************************/
s32 bsp_afc_protect_mutex_init(void)
{
    s32 s32ret = 0;
    s32ret =pthread_mutex_init(&g_mp_afc_protect, NULL);
    return s32ret;
}

/*******************************************************************************
* 函数名称: afc_log_init
* 功    能: afc
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态值，成功(BSP_OK)或失败
* 说   明:
*******************************************************************************/
u32  afc_log_init(void)
{
    DIR* pdir = opendir("/mnt/csi");
    if( NULL == pdir)//如果打不开文件夹，则重建一个
    {
        if(system("mkdir /mnt/csi") < 0)
        {
            printf("system call mkdir /mnt/csi wrong!\n");
            return BSP_ERROR;
        }
    }
    else
    {
        closedir(pdir);
    }
    return  BSP_OK;
}

/*******************************************************************************
* 函数名称: afc_log_init
* 功    能: afc
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态值，成功(BSP_OK)或失败
* 说   明:
*******************************************************************************/

u32  afc_log_out(void)
{
    u32 i;
    u32 file_size=0;
    u32 count= 0;
    FILE *fp = fopen(AFC_LOG_PATH, "w");
    if (fp == NULL )
    {
        printf( "get /mnt/csi/afc.log non-existent! \n");
        return BSP_ERROR;
    }
    file_size = sizeof(afc_log);
    for(i=0; i<AFC_LOG_LENTH; i++)
    {
        fprintf(fp,"%s",afc_log[i]);
    }
    fflush(fp);
    fclose(fp);
}

/*******************************************************************************
* 函数名称: bsp_afc_init
* 功    能: afc模块初始化函数
* 相关文档:
* 函数类型: s32
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32 bsp_afc_init(void)
{
    s32 s32ret = 0;
    printf("loading bsp_afc_init()!\r\n");
    pthread_attr_t attr_afc_main,attr_afc_get_phase;
    struct sched_param param_afc_main, param_afc_get_phase;
    memset((void *)&g_struAfcStat, 0x0, sizeof(g_struAfcStat));
    memset((void *)&g_struAfcCounter, 0x0, sizeof(g_struAfcCounter));
    memset((void *)&g_struAfcPhase, 0x0, sizeof(g_struAfcPhase));
    memset((void *)&g_struAfcFrq, 0x0, sizeof(g_struAfcFrq));
    memset((void *)&g_struAfcComp, 0x0, sizeof(g_struAfcComp));

    if (AFC_DD_NOT_INITED != g_struAfcStat.SpiInit)
    {
        return E_AFC;
    }

    g_struAfcStat.SpiInit = AFC_DD_INITED;

    g_u32Afcprint_level = AFC_PRINT_LEVEL_NORMAL;
    g_struAfcFrq.Cnt20sHisLen = 1;
    g_struAfcDac.DacLastTime = DEFAULT_DAC_VALUE;
    g_struAfcDac.DacValue = DEFAULT_DAC_VALUE;
    g_struAfcPhase.PhaseAdjFlag = PHASE_MODULATION_DISABLE;
    g_struAfcFrq.FrqAdjFlag = FREQUENCE_MODULATION_DISABLE;
    g_struAfcComp.Temperature.u8TempNum = 1;

    bsp_afc_cpld_mutex_init();
    bsp_afc_protect_mutex_init();

    sem_init(&g_semb_afc_main_proc,0,0);
    sem_init(&g_semb_getphas_sendac,0,0);
    pthread_attr_init(&attr_afc_main);
    pthread_attr_setinheritsched(&attr_afc_main, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr_afc_main, SCHED_RR);
    param_afc_main.sched_priority = 40;
    pthread_attr_setschedparam(&attr_afc_main, &param_afc_main);

    afc_log_init();

    s32ret = pthread_create(&t_afc_func_thread, &attr_afc_main, (void *)t_afc_func, 0);
    if (s32ret < 0)
    {
        afc_dbg( "t_afc_func task create failed!\n");
        return E_AFC;
    }

    pthread_attr_init(&attr_afc_get_phase);
    pthread_attr_setinheritsched(&attr_afc_get_phase, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr_afc_get_phase, SCHED_RR);
    param_afc_get_phase.sched_priority = 41;
    pthread_attr_setschedparam(&attr_afc_get_phase, &param_afc_get_phase);
    s32ret = pthread_create(&t_afc_get_freq_phase_thread, &attr_afc_get_phase, (void *)t_afc_get_freq_phase, 0);
    if (s32ret < 0)
    {
        afc_dbg( "t_afc_get_freq_phase task create failed!\n");
        return E_AFC;
    }

    if(AFC_PRINT_LEVEL_DEBUG < g_u32Afcprint_level)
    {
        afc_dbg( "INFO: AFC DD INITED .\n");
    }

    return BSP_OK;

}

/*******************************************************************************
* 函数名称:  afc_phasedif_avg
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
* 返回值:
* 无
* 说   明:
*******************************************************************************/
double  afc_phasedif_avg(u32 u32lenth)
{
    u8 u8i;
    double sum = 0.0;
    double result = 0.0;

    pthread_mutex_lock(&g_mp_afc_protect);
    for (u8i=0; u8i<(u32lenth - 1); u8i++)
    {
        g_struAfcPhase.PhaseDifGrp[u8i] = g_struAfcPhase.PhaseDifGrp[u8i+1];
        sum += g_struAfcPhase.PhaseDifGrp[u8i];
    }
    g_struAfcPhase.PhaseDifGrp[u32lenth - 1] = g_struAfcPhase.PhaseDif;
    sum += g_struAfcPhase.PhaseDifGrp[u32lenth -1];
    result = sum / u32lenth;
    pthread_mutex_unlock(&g_mp_afc_protect);

    return result;

}

/*******************************************************************************
* 函数名称: afc_freqdif_avg
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
*******************************************************************************/
double afc_freqdif_avg(const vu32 *pu32Array, u8 u8CurHis)
{
    u32 u32Array[32];
    double Sum = 0.0;
    double result = 0.0;
    u8  u8i = 0;

    memcpy((char *)u32Array, (char *)pu32Array, u8CurHis*sizeof(s32));
    for(u8i = 0; u8i < u8CurHis; u8i++)
    {
        Sum += u32Array[u8i];
    }
    result = (double)Sum / (20*u8CurHis) - COUNTER_61M44;
    //printf("20s频率计数值 = %f, 20s平均频率差 = %f \n",Sum,result);
    return result;
}

/*******************************************************************************
* 函数名称: afc_check_ocxo
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32 afc_check_ocxo(void)
{
    (void)spi_dac_write(0,MIN_DAC_VALUE);
    sleep(5);

    if ((CntDif1s_min=g_struAfcFrq.CntDif1s) > 0)
    {
        afc_dbg( "******************** ERROR : ocxo is bad *******************\n");
        return E_AFC;
    }

    (void)spi_dac_write(0,MAX_DAC_VALUE);
    sleep(5);

    if ((CntDif1s_max = g_struAfcFrq.CntDif1s) < 0)
    {
        afc_dbg( "******************** ERROR : ocxo is bad *******************\n");
        return E_AFC;
    }

    g_struAfcDac.DacValue = DEFAULT_DAC_VALUE;
    (void)spi_dac_write(0,DEFAULT_DAC_VALUE);
    sleep(5);

    afc_dbg( "************************* OCXO is OK ************************\n");
    return AFC_OK;
}

/*******************************************************************************
* 函数名称: afc_get_OCcoef
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
void afc_get_OCcoef(u16 u16lowDAC, u16 u16higDAC, float *padjCoef)
{
    s32 dif_pre = 0;
    s32 dif_cur = 0;
    float adjcoef = 0.0;

    spi_dac_write(0,u16lowDAC);
    sleep(10);
    dif_pre = g_struAfcFrq.CntDif1s;
    afc_dbg("**********写 0x%x 时，1s 频率计数差 = %d **************\n",u16lowDAC,dif_pre);
    spi_dac_write(0,u16higDAC);
    sleep(10);
    dif_cur = g_struAfcFrq.CntDif1s;
    afc_dbg("**********写 0x%x 时，1s 频率计数差 = %d **************\n",u16higDAC,dif_cur);

    pthread_mutex_lock(&g_mp_afc_protect);
    if((dif_pre >= 0) || (dif_cur <= 0))
    {
        adjcoef = 266;
        //printf("---- Warning:dif_pre or dif_cur error, ocxo adj coef default value!----\n");
    }
    else
    {
        adjcoef = ((float)(u16higDAC - u16lowDAC)) / (dif_cur - dif_pre);
        adjcoef = adjcoef * (float)0.8;

        if((adjcoef > 900.0) || (adjcoef < 30.0))
        {
            adjcoef = 266;
            //printf("---- Warning:ocxo adj coef calculation error, default value!----\n");
        }
    }
    pthread_mutex_unlock(&g_mp_afc_protect);

    *padjCoef = adjcoef;
}

/*******************************************************************************
* 函数名称:  afc_round
* 功    能:
* 相关文档:
* 函数类型: s32
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
double  afc_round(double data)
{
    double a;

    if(data == 0)
    {
        return data;
    }

    if (data > 0)
    {
        a = data - (u32)data;
        if (a >= 0.5)
        {
            return (double)((u32)data + 1);
        }
        else
        {
            return (double)((u32)data);
        }
    }
    else
    {
        a = (s32)data - data;
        if (a >= 0.5)
        {
            return (double)((s32)data -1);

        }
        else
        {
            return (double)((s32)data);
        }
    }
}

/*******************************************************************************
* 函数名称:  afc_freq_modulate_prepare
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
*******************************************************************************/
void  afc_freq_modulate_prepare(void)
{
    if(bts_synsrc_type == SYNSRC_LOCAL)
    {
        s32Value20scur=s32Value20s_end=s32Value20slast=u32Value20sfirstcnt=u32en20s = 0;
        memset(s32Value20stotal,0,CNT_WINDOWS);
        return;
    }

    afc_freq_count_20s(&s32Value20scur);

    if (s32Value20slast==s32Value20scur)
    {
        if (0 == gps_check_clock())
        {
            //printf("---- Warning:current frequency difference counter is equvalent to the last one!----\n");
        }
        return;
    }

    s32Value20slast = s32Value20scur;

    if (u32en20s == 1)
    {
        s32Value20stotal[0]=s32Value20s_end;
        u32Value20sfirstcnt = 1;
    }

    if(u32Value20sfirstcnt <20)
    {
        s32Value20stotal[u32Value20sfirstcnt]=s32Value20scur;
        u32Value20sfirstcnt++;
        u32en20s = 0;
    }
    else
    {
        s32Value20stotal[u32Value20sfirstcnt]=s32Value20scur;
        s32Value20s_end = s32Value20scur;
        u32Value20sfirstcnt = 0;
        u32en20s = 1;
        //g_first20sfrqcnt_full_flag = 1;
    }


    if ( 1 == u32en20s)
    {
        if (s32Value20stotal[20]  > s32Value20stotal[0])
        {
            g_struAfcFrq.Cnt20s = s32Value20stotal[20]  - s32Value20stotal[0];
        }
        else
        {
            g_struAfcFrq.Cnt20s = s32Value20stotal[20]  + 0x80000000 - s32Value20stotal[0];
        }

        g_struAfcFrq.Cnt20sGrp[0] = g_struAfcFrq.Cnt20s;

        pthread_mutex_lock(&g_mp_afc_protect);
        g_struAfcFrq.CntDifAvg = afc_freqdif_avg(g_struAfcFrq.Cnt20sGrp, 1);
        g_struAfcFrq.FrqAdjFlag = FREQUENCE_MODULATION_ENABLE;
        pthread_mutex_unlock(&g_mp_afc_protect);

        if ( 1 == g_u32afc_print )
        {
            int i;
            printf("first = %d, last = %d , 20s频率计数值 = %d \n",s32Value20stotal[0],s32Value20stotal[20],g_struAfcFrq.Cnt20s);

            for(i=0; i<21; i++)
            {
                printf("%d = %d \n",i,s32Value20stotal[i]);
            }
        }
    }
}

/*******************************************************************************
* 函数名称: afc_freq_modulate
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
*******************************************************************************/
s32 afc_freq_modulate(void)
{
    s32 s32Ret;
    s32 s32dac = 0;

    if (PHASE_MODULATION_DISABLE == g_struAfcPhase.PhaseAdjFlag)
    {
        if (g_struAfcFrq.FrqAdjFlag == FREQUENCE_MODULATION_ENABLE)
        {

            pthread_mutex_lock(&g_mp_afc_protect);
            s32dac = (s32) afc_round(g_struAfcDac.DacLastTime - (g_struAfcFrq.OCCoef * g_struAfcFrq.CntDifAvg));
            if (s32dac >= (s32)MAX_DAC_VALUE)
            {
                g_struAfcDac.DacValue =  MAX_DAC_VALUE;
            }
            else if (s32dac <= 0)
            {
                g_struAfcDac.DacValue =  0;
            }
            else
            {
                g_struAfcDac.DacValue = (u16)s32dac;
            }

            g_struAfcDac.DacLastTime = g_struAfcDac.DacValue;
            pthread_mutex_unlock(&g_mp_afc_protect);

            s32Ret = spi_dac_write(0, g_struAfcDac.DacValue);
            if (AFC_OK != s32Ret)
            {
                afc_dbg( "ERROR : LINE %d  spi_dac_write ! s32Ret = %d\n", __LINE__, s32Ret);
            }

            if(AFC_LOCK == g_struAfcStat.PhaseLockStage || AFC_HOLDOVER == g_struAfcStat.PhaseLockStage || AFC_HOLDOVER_TIMEOUT == g_struAfcStat.PhaseLockStage)
            {
                if(AFC_PRINT_LEVEL_NORMAL < g_u32Afcprint_level)
                {
                    afc_dbg( "*********************调频DAC 0x%x********************\n",g_struAfcDac.DacValue);
                }
            }
            else
            {
                pthread_mutex_lock(&g_mp_spi_dd_phase_epld);
                afc_cpld_clk_rst_oc();
                pthread_mutex_unlock(&g_mp_spi_dd_phase_epld);

                if(AFC_PRINT_LEVEL_NORMAL < g_u32Afcprint_level)
                {
                    afc_dbg( "*********************调频DAC 0x%x********************\n",g_struAfcDac.DacValue);
                }
            }
#if 1
            if (abs(g_struAfcFrq.CntDif1s) < AFC_FREQ_DIF_REF
                    && (g_struAfcFrq.CntDifAvg > (-AFC_FREQ_DIF_AVG_REF))
                    && (g_struAfcFrq.CntDifAvg < AFC_FREQ_DIF_AVG_REF)
                    && abs(g_struAfcPhase.PhaseDif) < AFC_PHASE_DIF_REF)
            {
                memset((void *)g_struAfcPhase.PhaseDifGrp,0x0,sizeof(g_struAfcPhase.PhaseDifGrp));
                g_struAfcPhase.PhaseDifAvg = 0;
                g_struAfcPhase.PhaseAdjFlag = PHASE_MODULATION_ENABLE;

                afc_dbg( "****************   调频--->调相 ******************\n");

                g_struDACSave.DacValue = g_struAfcDac.DacValue;
                g_struDACSave.OCCoef  = g_struAfcFrq.OCCoef;

                afc_dbg( "***DAC %d***  OCXOEF  %f*****************\n",g_struAfcDac.DacValue,g_struAfcFrq.OCCoef);

                afc_dac_save((char *)&g_struDACSave);
            }
#endif
        }

    }

    return AFC_OK;
}
/*******************************************************************************
* 函数名称: afc_phase_modulate
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32 afc_phase_modulate(void)
{
    s32 s32Ret;
    s32 s32dac = 0;
    u32 u32threshold=0;

    if (PHASE_MODULATION_ENABLE == g_struAfcPhase.PhaseAdjFlag)
    {
        if (g_struAfcPhase.PhaseDifAvg > 1)
        {
            if (g_struAfcPhase.PhaseDifAvg > 10)
            {
                g_struAfcDac.DacLastTime--;
            }
            else
            {
                g_u32AfcDacCnt1++;

                if (g_u32AfcDacCnt1 > 4)
                {
                    g_u32AfcDacCnt1 = 0;
                    g_struAfcDac.DacLastTime--;
                }
            }
        }
        else
        {
            g_u32AfcDacCnt1 = 0;
        }

        if (g_struAfcPhase.PhaseDifAvg < -1)
        {
            if (g_struAfcPhase.PhaseDifAvg < -10)
            {
                g_struAfcDac.DacLastTime++;
            }
            else
            {
                g_u32AfcDacCnt2++;

                if (g_u32AfcDacCnt2 > 4)
                {
                    g_u32AfcDacCnt2 = 0;
                    g_struAfcDac.DacLastTime++;
                }
            }
        }
        else
        {
            g_u32AfcDacCnt2 = 0;
        }

        pthread_mutex_lock(&g_mp_afc_protect);
        s32dac = (s32) afc_round(g_struAfcDac.DacLastTime - 16*(g_struAfcPhase.PhaseDifAvg));
        if (s32dac >= (s32)MAX_DAC_VALUE)
        {
            g_struAfcDac.DacValue =  MAX_DAC_VALUE;
        }
        else if (s32dac <= 0)
        {
            g_struAfcDac.DacValue =  0;
        }
        else
        {
            g_struAfcDac.DacValue = (u16)s32dac;
        }

        pthread_mutex_unlock(&g_mp_afc_protect);

        s32Ret = spi_dac_write(0, g_struAfcDac.DacValue);
        if (AFC_OK != s32Ret)
        {
            afc_dbg( "ERROR : LINE %d  spi_dac_write ! s32Ret = %d\n", __LINE__, s32Ret);
        }
    }

    return AFC_OK;
}

/*******************************************************************************
* 函数名称:  afc_cpld_clk_rst_oc
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
* 返回值:
* 无
* 说   明:
*******************************************************************************/
void  afc_cpld_clk_rst_oc(void)
{
    if (g_Clock_Set_AllDate.gps_Type == 0x6)      //ES外同步
    {
        bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0xe0);
        usleep(1000*120);
        bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0xc0);
	 return;
    }
    if (g_Clock_Set_AllDate.gps_Type != 0xFF)
    {
        bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x20);
        usleep(1000*120);
        bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x00);
    }
    else
    {
        bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x20);
        usleep(1000*120);
        bsp_cpld_write_reg(CPLD_AFC_CTRL_REG,0x40);
    }
}

/*******************************************************************************
* 函数名称:  afc_check_locked
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32  afc_check_locked(void)
{
    if (abs(g_struAfcFrq.CntDif1s) < AFC_LOCK_FREDIF_REF
            &&  g_struAfcFrq.CntDifAvg < AFC_LOCK_FREDIF_AVG_REF
            &&  g_struAfcFrq.CntDifAvg > (-AFC_LOCK_FREDIF_AVG_REF)
            &&  abs(g_struAfcPhase.PhaseDif) < AFC_LOCK_PHASEDIF_REF
            &&  g_struAfcPhase.PhaseDifAvg < AFC_LOCK_PHASEDIF_AVG_REF
            &&  g_struAfcPhase.PhaseDifAvg >(-AFC_LOCK_PHASEDIF_AVG_REF))
    {
        return AFC_OK;
    }
    else
    {
        return E_AFC;
    }
    return AFC_OK;
}
/*******************************************************************************
* 函数名称:  afc_state_change
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32  afc_state_change(u8 u8changeto)
{
    if(u8changeto != AFC_LOCK)
    {
        g_gps_lock = 0;
    }
    if (u8changeto > AFC_STATE_MAXVALUE)
    {
        return AFC_ERROR;
    }

    g_struAfcStat.PhaseLockStage = u8changeto;

    afc_gps_led_control();

    return AFC_OK;
}

/*******************************************************************************
* 函数名称:  afc_comp_coef_cal
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称           类型        输入/输出       描述
*******************************************************************************/
void  afc_comp_coef_cal(u32 u32datalen)
{
    u32 u32i;
    u32 datalen = u32datalen;
    char s8str[100];

    double tt[AFC_COMP_TIME_LENGTH*3]= {0};
    double tt_trans[AFC_COMP_TIME_LENGTH*3]= {0};
    double mul1[9]= {0};
    double mul2[AFC_COMP_TIME_LENGTH*3]= {0};
    double coef[3]= {0};

    pthread_mutex_lock(&g_mp_afc_protect);

    for(u32i=0; u32i<datalen; u32i++)
    {
        tt[3*u32i]  = g_struAfcComp.TemperatureGrp[u32i];
        tt[3*u32i+1] = u32i+1;
        tt[3*u32i+2] = 1;
    }

    afc_matrix_transpose(tt,(s32)datalen,3,tt_trans);
    afc_matrix_mul(tt_trans,tt,(s32)3,(s32)datalen,(s32)3,mul1);
    afc_matrix_inver(mul1,(s32)3);
    afc_matrix_mul(mul1,tt_trans,(s32)3,(s32)3,(s32)datalen,mul2);
    afc_matrix_mul(mul2,g_struAfcComp.DACGrp,(s32)3,(s32)datalen,(s32)1,coef);

    g_struAfcComp.TemperatureCoef = coef[0];
    g_struAfcComp.TimeCoef  = coef[1];
    g_struAfcComp.DACCoef  = coef[2];

    pthread_mutex_unlock(&g_mp_afc_protect);

    if(AFC_PRINT_LEVEL_NORMAL < g_u32Afcprint_level)
    {
        (void)sprintf(s8str, "      温度参数= %f   时间参数= %f   中心DAC    = %f  \n",
                      g_struAfcComp.TemperatureCoef, g_struAfcComp.TimeCoef, g_struAfcComp.DACCoef);

        afc_dbg("***********************************\n");
        afc_dbg("%s",s8str);
        afc_dbg("***********************************\n");
    }
}

/*******************************************************************************
* 函数名称: afc_comp_coef_adjust
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称           类型        输入/输出       描述
*******************************************************************************/
void afc_comp_coef_adjust(u32 u32datalen)
{
    u32 u32i=0;
    u32 u32len;
    double dac_simu[20];
    double dac_dif[20];
    double sum=0.0;
    char s8str1[50];

    pthread_mutex_lock(&g_mp_afc_protect);
    for (u32len = u32datalen-20; u32len < u32datalen; u32len++)
    {
        dac_simu[u32i] =  g_struAfcComp.TemperatureGrp[u32len]*g_struAfcComp.TemperatureCoef
                          + (double)(u32len)*g_struAfcComp.TimeCoef + g_struAfcComp.DACCoef;
        dac_dif[u32i] = dac_simu[u32i] - g_struAfcComp.DACGrp[u32len];
        sum += dac_dif[u32i];
        u32i++;
    }

    g_struAfcComp.DACCoef_adjust = sum/20;
    g_struAfcComp.DACCoef = g_struAfcComp.DACCoef - g_struAfcComp.DACCoef_adjust;

    pthread_mutex_unlock(&g_mp_afc_protect);
    if(AFC_PRINT_LEVEL_NORMAL < g_u32Afcprint_level)
    {
        sprintf(s8str1, "       校正后中心DAC = %f  \n", g_struAfcComp.DACCoef);
        afc_dbg("***********************************\n");
        afc_dbg("%s",s8str1);
        afc_dbg("***********************************\n");
    }
}

/*******************************************************************************
* 函数名称:  afc_comp_func
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称           类型        输入/输出       描述
* 返回值:
* 说   明:
*******************************************************************************/
void  afc_comp_func(void)
{
    s32 s32dac = 0;
    s32 ret=0;

    pthread_mutex_lock(&g_mp_afc_protect);
    s32dac = (s32) afc_round(g_struAfcComp.Temperature.s16TempValue[1]*g_struAfcComp.TemperatureCoef
                             +  g_struAfcComp.CompCnt*g_struAfcComp.TimeCoef
                             +  g_struAfcComp.DACCoef);

    if (s32dac >= (s32)MAX_DAC_VALUE)
    {
        g_struAfcDac.DacValue = MAX_DAC_VALUE;
    }
    else if (s32dac <= 0)
    {
        g_struAfcDac.DacValue = 0;
    }
    else
    {
        g_struAfcDac.DacValue = (u16)s32dac;
    }

    pthread_mutex_unlock(&g_mp_afc_protect);

    ret = spi_dac_write(0, g_struAfcDac.DacValue);
    if (AFC_OK != ret)
    {
        afc_dbg( "ERROR : LINE %d  spi_dac_write ! s32Ret = %d\n", __LINE__, ret);
    }

    if(AFC_PRINT_LEVEL_NORMAL < g_u32Afcprint_level)
    {
        afc_dbg("[%d]------- 温度=  %d       补偿DAC  =  %d\n", (s32)g_struAfcComp.CompCnt, g_struAfcComp.Temperature.s16TempValue[1], g_struAfcDac.DacValue);
    }
}

/*******************************************************************************
* 函数名称: afc_dac_save
* 功    能: afc
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态值，成功(BSP_OK)或失败
* 说   明:
*******************************************************************************/
void afc_dac_save(char  *dacsave)
{
    int count;

    FILE *fp = fopen(AFC_DAC_PATH, "w");
    if (fp == NULL )
    {
        printf( "save /mnt/csi/dac.txt non-existent! \n");
        return ;
    }
    count = fwrite((char *)dacsave, 1, 8, fp);
    if(count != 8)
    {
        printf( "afc_dac_save ERROR! \n");
    }
    else
    {
        printf( "afc_dac_save ok! \n");
    }
    fflush(fp);
    fclose(fp);
}
/*******************************************************************************
* 函数名称: afc_dac_get
* 功    能: afc
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态值，成功(BSP_OK)或失败
* 说   明:
*******************************************************************************/
u32  afc_dac_get(STRU_AFC_DAC_SAVE *dacvalue)
{
    int count;

    FILE *fp = fopen(AFC_DAC_PATH, "r");
    if (fp == NULL )
    {
        printf( "get /mnt/csi/dac.txt non-existent! \n");
        return BSP_ERROR;
    }
    count = fread((char *)dacvalue, 1, 8, fp);
    if(count != 8)
    {
        printf( "afc_dac_get ERROR! \n");
    }
    else
    {
        printf( "afc_dac_get ok! \n");
    }
    fclose(fp);

    return  BSP_OK;
}

void afc_gps_led_flicker()
{
    static int led = 0;
    if(led == 0)
        bsp_led_gps_off();
    if(led == 1)
        bsp_led_gps_on();
    led = 1 - led;
}

void afc_gps_led_flicker_on()
{
    bsp_led_gps_off();
    usleep(100000);
    bsp_led_gps_on();
    usleep(100000);
}

static void afc_gps_led_control()
{
    if(g_struAfcStat.PhaseLockStage < AFC_LOCK)
        bsp_led_gps_off();
    else if(g_struAfcStat.PhaseLockStage == AFC_LOCK)
        bsp_led_gps_on();
    else if(g_struAfcStat.PhaseLockStage == AFC_HOLDOVER)
        bsp_led_gps_blink(1000);
    else if(g_struAfcStat.PhaseLockStage == AFC_HOLDOVER_TIMEOUT
            || g_struAfcStat.PhaseLockStage == AFC_ABNORMAL)
        bsp_led_gps_blink(100);
}

u16 afc_lock_status_get()
{
    /*lock状态:
     *1、<3:自由振荡(0:startup 1:warmup 2:fast)
     *2、=3:锁定/同步(lock)
     *3、=4:保持(holdover)
     *4、=5:保持超时(holdover timeout)
     *5、=6:异常(abnormal)
     */
    if ((g_Clock_Set_AllDate.gps_Type == 1) && (g_Clock_Set_AllDate.gps_ClockSynWay == 1) && (g_Clock_Set_AllDate.gps_CascadeCfg == 0))
    {
        return 3;
    }
    if ((g_ublox_gps_type == 0) && (g_GNSS_flag == 2))
    {
        return 1;
    }
    return g_struAfcStat.PhaseLockStage;
}

void afc_syn_src()
{
	switch(bsp_get_current_clocksource())
	{
		case SYNC_SOURCE_GPS: 	bts_synsrc_type = SYNSRC_GPS; break;
		case SYNC_SOURCE_LOCAL: bts_synsrc_type = SYNSRC_LOCAL; break;
		case SYNC_SOURCE_BD: 	bts_synsrc_type = SYNSRC_BD; break;
		case SYNC_SOURCE_1PPS_TOD: bts_synsrc_type = SYNSRC_1PPS_TOD; break;
		case SYNC_SOURCE_1588: 	bts_synsrc_type = SYNSRC_1588; break;
		case SYNC_SOURCE_ES: 	bts_synsrc_type = SYNSRC_ES; break;
		default: break;
	}
}

BOOL isfloatzero(float a)
{
    if(a >= -0.000001 && a <= 0.000001)
        return TRUE;
    else
        return FALSE;
}
/*******************************************************************************
* 函数名称: t_ afc_func
* 功    能:
* 相关文档:
* 函数类型: s32
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32 t_afc_func(void)
{
    u32 u32i;
    u32 gps_st_count=0;
    u32 u32phasechangecount = 0;
    u32 u32compensatecount = 0;
    u32 u32GpsClkAvailableCount = 0;
    u32 u32GpsClkUnAvailableCount = 0;
    u16 u16LockTimes = 0;
    u16 u16unlockTimes = 0;
    u16 u16FastLockTimes = 0;
    u16 u16FastunlockTimes = 0;
    u16 u16GpsAvailableCountInHoldver = 0;
    //static int dwdspload=0;
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    afc_syn_src();

    for(;;)
    {
        sem_wait(&g_semb_afc_main_proc);
        /*本地同步源，afc不做调整*/
        if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
        {
            g_struAfcDac.DacValue = DEFAULT_DAC_VALUE;
            //(void)spi_dac_write(0,DEFAULT_DAC_VALUE);
            continue;
        }
        /*=============================AFC_STARTUP===========================================*/
        if(AFC_STARTUP == g_struAfcStat.PhaseLockStage)
        {
            g_struAfcCounter.HoldoverCnt = 0;
            interpro_syn_status_change_notify(AFC_STARTUP);
            afc_dbg( "\n ******************** AFC  BEGIN START-UP ********************\n");

            g_struResetReason.u8ResetCause = bsp_get_resetcause();
            afc_dbg( "u8ResetCause ＝0x%x\n",g_struResetReason.u8ResetCause);
            //bsp_dbg( "u8ResetCause ＝0x%x\n",g_struResetReason.u8ResetCause);

            if(POWER_ON_RESET_MASK ==  g_struResetReason.u8ResetCause)
            {
                afc_dbg( "******************** AFC_STARTUP ---> AFC_WARMUP ********************\n");
                sprintf(alarm_afc_status_change,"AFC status AFC_STARTUP change to AFC_WARMUP");
                //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_STATUS_CHANGE_ALARM,BSP_ALM_CLASS_INFO,alarm_afc_status_change,sizeof(alarm_afc_status_change),0);
                //            interpro_syn_status_change_notify(AFC_WARMUP);
                (void) afc_state_change(AFC_WARMUP);
            }
            else
            {
                //g_u8ResetCause = 1;
                for(;;)
                {
                    /*本地同步源，afc不做调整*/
                    if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                    {
                        break;
                    }
                    u32GpsClkAvailableCount = 0;
                    for(u32i=0; u32i<AFC_STARTUP_GPS_AVAILABLE_TIMES_MIN; u32i++)
                    {
                        sem_wait(&g_semb_afc_main_proc);
                        if(AFC_OK ==  gps_check_clock())
                        {
                            u32GpsClkAvailableCount++;
                            afc_dbg( "Time Source Ready = %d!\n",(int)u32GpsClkAvailableCount);
                        }
                        else
                        {
                            u32GpsClkAvailableCount = 0;
                            break;
                        }
                    }

                    if(AFC_STARTUP_GPS_AVAILABLE_TIMES_MIN == u32GpsClkAvailableCount)
                    {
                        afc_dbg( "Time Source is usable !\n");
                        afc_dbg( "******************** AFC_STARTUP ---> AFC_FAST ********************\n");
                        interpro_syn_status_change_notify(AFC_FAST);
                        (void) afc_state_change(AFC_FAST);

                        pthread_mutex_lock(&g_mp_spi_dd_phase_epld);
                        afc_cpld_clk_rst_oc();
                        pthread_mutex_unlock(&g_mp_spi_dd_phase_epld);
                        break;
                    }
                }
            }
        }

        /*=============================AFC_WARMUP===========================================*/
        if(AFC_WARMUP == g_struAfcStat.PhaseLockStage)
        {
            afc_dbg( "******************** OCXO 预热2 分钟 ********************\n");
            (void)spi_dac_write(0,DEFAULT_DAC_VALUE);
            for (u32i = 0; u32i < 120; u32i++)
            {
                sem_wait(&g_semb_afc_main_proc);
                //sleep(1);
            }

            for(;;)
            {
                /*本地同步源，afc不做调整*/
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }
                u32GpsClkAvailableCount = 0;
                for(u32i=0; u32i<AFC_WARMUP_GPS_AVAILABLE_TIMES_MIN; u32i++)
                {
                    sem_wait(&g_semb_afc_main_proc);
                    if(AFC_OK ==  gps_check_clock())
                    {
                        u32GpsClkAvailableCount++;
                        afc_dbg( "Time Source Ready = %d!\n",(int)u32GpsClkAvailableCount);
                    }
                    else
                    {
                        u32GpsClkAvailableCount = 0;
                        break;
                    }
                }

                if(AFC_WARMUP_GPS_AVAILABLE_TIMES_MIN == u32GpsClkAvailableCount)
                {
                    afc_dbg( "Time Source is usable !\n");
                    afc_dbg( "******************** AFC_WARMUP ---> AFC_FAST ********************\n");
                    sprintf(alarm_afc_status_change,"AFC status AFC_WARMUP change to AFC_FAST");
                    //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_STATUS_CHANGE_ALARM,BSP_ALM_CLASS_INFO,alarm_afc_status_change,sizeof(alarm_afc_status_change),1);
                    //              interpro_syn_status_change_notify(AFC_FAST);
                    (void) afc_state_change(AFC_FAST);

                    pthread_mutex_lock(&g_mp_spi_dd_phase_epld);
                    afc_cpld_clk_rst_oc();
                    pthread_mutex_unlock(&g_mp_spi_dd_phase_epld);

                    break;
                }
            }
        }

        /*=============================AFC_FAST===========================================*/
        if(AFC_FAST == g_struAfcStat.PhaseLockStage)
        {
            u16LockTimes = 0;
            u16unlockTimes = 0;

            if (POWER_ON_RESET_MASK == g_struResetReason.u8ResetCause)
            {
                afc_dbg("********************** 断电复位**************************\n");
            }
            else
            {
                afc_dbg("********************** 带电复位**************************\n");
            }

            if (BSP_OK==afc_dac_get(&g_struDACSave))
            {
                g_struAfcDac.DacValue = g_struDACSave.DacValue;
                g_struAfcFrq.OCCoef  = g_struDACSave.OCCoef;
                if(isfloatzero(g_struAfcFrq.OCCoef))
                    afc_get_OCcoef(DAC_VALUE_REF_LOW, DAC_VALUE_REF_HIGH, &(g_struAfcFrq.OCCoef));
            }
            else
            {
                afc_get_OCcoef(DAC_VALUE_REF_LOW, DAC_VALUE_REF_HIGH, &(g_struAfcFrq.OCCoef));
                g_struAfcDac.DacValue = DEFAULT_DAC_VALUE;
            }

            if (g_struAfcDac.DacValue > DAC_VALUE_REF_HIGH || g_struAfcDac.DacValue < DAC_VALUE_REF_LOW)
            {
                g_struAfcDac.DacValue = DEFAULT_DAC_VALUE;
            }


            afc_dbg("****************** 调节系数 = %f   ****************\n", g_struAfcFrq.OCCoef);
            afc_dbg("****************** 默认DAC      = %d ****************\n", g_struAfcDac.DacValue);

            g_struAfcDac.DacLastTime = g_struAfcDac.DacValue;
            (void)spi_dac_write(0,g_struAfcDac.DacValue);

            pthread_mutex_lock(&g_mp_spi_dd_phase_epld);
            afc_cpld_clk_rst_oc();
            pthread_mutex_unlock(&g_mp_spi_dd_phase_epld);

            g_struAfcFrq.Cnt20sHisLen = 1;

            for (u32i = 0; u32i < 20; u32i++)
            {
                s32Value20stotal[u32i] = 0;
            }
            u32Value20sfirstcnt = 0;
            g_u32afc_20sfrqcnt_start = 1;


            for (;;)
            {
                sem_wait(&g_semb_afc_main_proc);
                /*本地同步源，afc不做调整*/
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }


                if(AFC_OK !=  gps_check_clock())
                {
                    gps_st_count++;
                    if(!(gps_st_count%30))
                    {
                        gps_st_count=0;
                        afc_dbg( "WARNING : Time Source is not usable in FAST stasus!\n");
                    }
                }
                else
                {
                    (void)afc_freq_modulate();
                    (void)afc_phase_modulate();

                    if (AFC_OK ==  afc_check_locked() && (PHASE_MODULATION_ENABLE == g_struAfcPhase.PhaseAdjFlag))
                    {
                        u16LockTimes++;
                        u16unlockTimes = 0;
                        u16FastLockTimes++;
                    }
                    else
                    {
                        u16LockTimes = 0;
                        u16unlockTimes++;
                        u16FastunlockTimes++;
                    }

                    if (u16LockTimes > AFC_FAST_GPS_AVAILABLE_TIMES_MIN)
                    {
                        afc_dbg( "******************** AFC_FAST ---> AFC_LOCK ********************\n");
                        printf("u16FastLockTimes = %d, u16FastunlockTimes = %d.\n",u16FastLockTimes,u16FastunlockTimes);
                        sprintf(alarm_afc_status_change,"AFC status AFC_FAST change to AFC_LOCK");
                        //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_STATUS_CHANGE_ALARM,BSP_ALM_CLASS_INFO,alarm_afc_status_change,sizeof(alarm_afc_status_change),2);
                        //   interpro_syn_status_change_notify(AFC_LOCK);
                        (void) afc_state_change(AFC_LOCK);

                        g_struDACSave.DacValue = g_struAfcDac.DacValue;
                        g_struDACSave.OCCoef  = g_struAfcFrq.OCCoef;
                        afc_dbg( "***DAC %d***  OCXOEF  %f*****************\n",g_struAfcDac.DacValue,g_struAfcFrq.OCCoef);
                        afc_dac_save((char *)&g_struDACSave);

                        (g_struAfcCounter.LockTimesCnt)++;

                        /*锁定之后获取经纬度*/
                        if(gps_NAV_PosLLH() != BSP_OK)
                        {
                            bsp_dbg("gps_NAV_PosLLH error!\n");
                        }
                        g_gps_lock = 1;
                        break;
                    }
                    if(u16unlockTimes > AFC_FAST_NOT_LOCK_TIMES_MAX)
                    {
                        printf("u16FastLockTimes = %d, u16FastunlockTimes = %d.\n",u16FastLockTimes,u16FastunlockTimes);
                        afc_dbg("******************** AFC_FAST ---> AFC_ABNORMAL ********************\n");
                        (void) afc_state_change(AFC_ABNORMAL);
                        break;
                    }
                }
            }
        }

        /*=============================AFC_LOCK===========================================*/
        if(AFC_LOCK == g_struAfcStat.PhaseLockStage)
        {
            for (;;)
            {
#if 0
                /*本地同步源，afc不做调整*/
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }
#endif
                sem_wait(&g_semb_afc_main_proc);
                if(AFC_OK !=  gps_check_clock())
                {
                    u32GpsClkUnAvailableCount++;
                    if (u32GpsClkUnAvailableCount >2)
                    {
                        u32GpsClkUnAvailableCount = 0;

                        if (1 == g_struAfcComp.LenthFlag)
                        {
                            g_struAfcComp.CalCnt = AFC_COMP_TIME_LENGTH;
                            g_struAfcComp.CompCnt = AFC_COMP_TIME_LENGTH;
                            afc_comp_coef_cal(AFC_COMP_TIME_LENGTH);
                            afc_comp_coef_adjust(AFC_COMP_TIME_LENGTH);
                        }
                        else
                        {
                            g_struAfcComp.CompCnt =  g_struAfcComp.CalCnt;
                            afc_comp_coef_cal(g_struAfcComp.CalCnt);
                            afc_comp_coef_adjust(g_struAfcComp.CalCnt);
                        }

                        afc_dbg( "******************** AFC_LOCK ---> AFC_HOLDOVER ********************\n");
                        sprintf(alarm_afc_status_change,"AFC status AFC_LOCK change to AFC_HOLDOVER");
                        //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_STATUS_CHANGE_ALARM,BSP_ALM_CLASS_INFO,alarm_afc_status_change,sizeof(alarm_afc_status_change),3);
                        //                 interpro_syn_status_change_notify(AFC_HOLDOVER);
                        (void) afc_state_change(AFC_HOLDOVER);
                        (g_struAfcCounter.HoldoverTimesCnt)++;
                        g_struAfcFrq.Cnt20sHisLen = 1;
                        break;

                    }
                }
                else
                {
                    if(AFC_OK !=  afc_check_locked())
                    {
                        (g_struAfcCounter.FastUnlockCnt)++;
                        //g_u32AfcPrintCnt = 1;
                        //g_struAfcFrq.Cnt20sHisLen = 2;
                    }
                    else
                    {
                        g_struAfcCounter.FastUnlockCnt = 0;
                        //g_u32AfcPrintCnt = 20;
                        //g_struAfcFrq.Cnt20sHisLen = 6;
                    }

                    if(AFC_FAST_NOT_LOCK_TIMES_MAX < g_struAfcCounter.FastUnlockCnt)
                    {
                        afc_dbg( "******************** AFC_LOCK ---> AFC_ABNORMAL ********************\n");
                        //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_ABNORMAL_ALARM,BSP_ALM_CLASS_CRITICAL,alarm_afc_abnormal,sizeof(alarm_afc_abnormal),0);
                        //                  interpro_syn_status_change_notify(AFC_ABNORMAL);
                        (void) afc_state_change(AFC_ABNORMAL);
                        break;
                    }

                    //(void)afc_freq_modulate();
                    (void)afc_phase_modulate();

                    if (PHASE_MODULATION_ENABLE == g_struAfcPhase.PhaseAdjFlag)
                    {
                        u32phasechangecount++;
                        if (u32phasechangecount >= AFC_COMP_INTERVAL)
                        {
                            u32phasechangecount = 0;
                            if (0 == g_struAfcComp.LenthFlag)
                            {
                                g_struAfcComp.DACGrp[g_struAfcComp.CalCnt] = (double)(g_struAfcDac.DacValue);
                                g_struAfcComp.TemperatureGrp[g_struAfcComp.CalCnt] = (double)g_struAfcComp.Temperature.s16TempValue[1];
                                (g_struAfcComp.CalCnt)++;
                                if (g_struAfcComp.CalCnt >= AFC_COMP_TIME_LENGTH)
                                {
                                    g_struAfcComp.LenthFlag = 1;
                                    afc_comp_coef_cal(AFC_COMP_TIME_LENGTH);
                                    afc_comp_coef_adjust(AFC_COMP_TIME_LENGTH);
                                }
                            }
                            else
                            {
                                memcpy((double *)(g_struAfcComp.DACGrp), (double *)(&(g_struAfcComp.DACGrp[1])), (AFC_COMP_TIME_LENGTH-1)*sizeof(double));
                                g_struAfcComp.DACGrp[AFC_COMP_TIME_LENGTH-1] = (double)(g_struAfcDac.DacValue);
                                memcpy((double *)(g_struAfcComp.TemperatureGrp), (double *)(&(g_struAfcComp.TemperatureGrp[1])), (AFC_COMP_TIME_LENGTH-1)*sizeof(double));
                                g_struAfcComp.TemperatureGrp[AFC_COMP_TIME_LENGTH-1] = (double)(g_struAfcComp.Temperature.s16TempValue[1]);
                                (g_struAfcComp.CalCnt)++;

                                if (AFC_COMP_TIME_LENGTH+AFC_COMP_UPDATE_LENGTH == g_struAfcComp.CalCnt)
                                {
                                    g_struAfcComp.CalCnt = AFC_COMP_TIME_LENGTH;
                                    afc_comp_coef_cal(AFC_COMP_TIME_LENGTH);
                                    afc_comp_coef_adjust(AFC_COMP_TIME_LENGTH);
                                }
                            }
                        }
                    }
                }
            }
        }

        /*=============================AFC_HOLDOVER===========================================*/
        if(AFC_HOLDOVER == g_struAfcStat.PhaseLockStage)
        {
            afc_dbg( "******************** Entered AFC_HOLDOVER status! ********************\n");
            for (;;)
            {
#if 0
                /*本地同步源，afc不做调整*/
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }
#endif
                sem_wait(&g_semb_afc_main_proc);
                (g_struAfcCounter.HoldoverCnt)++;
                if(AFC_OK !=  gps_check_clock())
                {
                    u16GpsAvailableCountInHoldver = 0;

                    if (1 == g_struAfcComp.LenthFlag)
                    {
                        u32compensatecount++;
                        if (u32compensatecount>=AFC_COMP_INTERVAL)
                        {
                            u32compensatecount = 0;
                            (g_struAfcComp.CompCnt)++;
                            afc_comp_func();
                        }
                    }

                    if (g_struAfcCounter.HoldoverCnt > AFC_HOLDOVER_TIME_MAX)
                    {
                        afc_dbg( "******************** AFC_HOLDOVER ---> AFC_HOLDOVER_TIMEOUT ********************\n");
                        //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_HOLDOVER_TIMEOUT_ALARM,BSP_ALM_CLASS_CRITICAL,alarm_afc_timeout,sizeof(alarm_afc_timeout),0);
                        //                     interpro_syn_status_change_notify(AFC_HOLDOVER_TIMEOUT);
                        (void) afc_state_change(AFC_HOLDOVER_TIMEOUT);
                        (g_struAfcCounter.HoldoverCnt) = 0;
                        g_struAfcCounter.HoldoverTimeoutTimesCnt ++;
                        break;
                    }
                }
                else
                {
                    u16GpsAvailableCountInHoldver++;

                    if (u16GpsAvailableCountInHoldver > 30)
                    {
                        u16GpsAvailableCountInHoldver = 30;

                        //	memset((void *)g_struAfcPhase.PhaseDifGrp,0x0,sizeof(g_struAfcPhase.PhaseDifGrp));
                        //g_struAfcPhase.PhaseDifAvg = 0;
                        afc_dbg( "AFC_HOLDOVER---Time Source is usable!\n");


                        u32GpsClkAvailableCount = 0;


                        for(u32i=0; u32i<5; u32i++)
                        {
                            sem_wait(&g_semb_afc_main_proc);
                            (g_struAfcCounter.HoldoverCnt)++;

                            if(AFC_OK ==  gps_check_clock())
                            {
                                u32GpsClkAvailableCount++;
                            }
                            else
                            {
                                u32GpsClkAvailableCount = 0;
                                break;
                            }
                        }

                        if(u32GpsClkAvailableCount == AFC_HOLDOVER_GPS_AVAILABLE_TIMES_MIN)
                        {
                            u16LockTimes = 0;
                            u16unlockTimes = 0;
                            g_struAfcPhase.PhaseAdjFlag = PHASE_MODULATION_ENABLE;

                            //memset((void *)g_struAfcPhase.PhaseDifGrp,0x0,sizeof(g_struAfcPhase.PhaseDifGrp));
                            //g_struAfcPhase.PhaseDifAvg = 0;

                            for (;;)
                            {
                                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                                {
                                    break;
                                }
                                sem_wait(&g_semb_afc_main_proc);
                                (g_struAfcCounter.HoldoverCnt)++;

                                (void)afc_phase_modulate();

                                if (AFC_OK ==  afc_check_locked() && AFC_OK ==  gps_check_clock())
                                {
                                    u16LockTimes++;
                                    u16unlockTimes = 0;
                                }
                                else
                                {
                                    u16LockTimes = 0;
                                    u16unlockTimes++;

                                    if (AFC_OK !=  gps_check_clock())
                                    {
                                        break;
                                    }
                                }
                                if (u16LockTimes > AFC_HOLDOVER_GPS_AVAILABLE_LOCK_TIMES_MIN)
                                {
                                    afc_dbg( "******************** AFC_HOLDOVER ---> AFC_LOCK ********************\n");
                                    sprintf(alarm_afc_status_change,"AFC status AFC_HOLDOVER change to AFC_LOCK");
                                    //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_STATUS_CHANGE_ALARM,BSP_ALM_CLASS_INFO,alarm_afc_status_change,sizeof(alarm_afc_status_change),4);
                                    //                        interpro_syn_status_change_notify(AFC_LOCK);
                                    (void) afc_state_change(AFC_LOCK);
                                    g_struAfcFrq.Cnt20sHisLen = 1;
                                    (g_struAfcCounter.LockTimesCnt)++;
                                    g_struAfcCounter.HoldoverCnt = 0;
                                    g_gps_lock = 1;
                                    break;
                                }

                                if (u16unlockTimes > AFC_HOLDOVER_NOT_LOCK_TIMES_MAX)
                                {
                                    afc_dbg( "******************** AFC_HOLDOVER ---> AFC_ABNORMAL ********************\n");
                                    //afc_log_out();//log output
                                    //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_AFC_ABNORMAL_ALARM,BSP_ALM_CLASS_CRITICAL,alarm_afc_abnormal,sizeof(alarm_afc_abnormal),1);
                                    //                         interpro_syn_status_change_notify(AFC_ABNORMAL);
                                    (void) afc_state_change(AFC_ABNORMAL);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
#if 0
        /*=============================AFC_HOLDOVER TIMEOUT==================================*/
        if(AFC_HOLDOVER_TIMEOUT == g_struAfcStat.PhaseLockStage)
        {
            afc_dbg( "Entered AFC_HOLDOVER_TIMEOUT status!\n");
#ifndef	BSP_DEBUG
            VOS_AFCHoldoverTimeoutRestartSystem();
#endif
            for(;;)
            {
                sem_wait(&g_semb_afc_main_proc);
                /*本地同步源，afc不做调整*/
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }
                if (1 == g_struAfcComp.LenthFlag)
                {
                    u32compensatecount++;
                    if (u32compensatecount>=AFC_COMP_INTERVAL)
                    {
                        u32compensatecount = 0;
                        (g_struAfcComp.CompCnt)++;
                        afc_comp_func();

                        if(AFC_PRINT_LEVEL_DEBUG < g_u32Afcprint_level)
                        {
                            afc_dbg( "AFC  HOLDOVER TIME OUT !\n");
                        }

                    }
                }
            }
        }
#else
        if(AFC_HOLDOVER_TIMEOUT == g_struAfcStat.PhaseLockStage)
        {
            afc_dbg( "******************** Entered AFC_HOLDOVER_TIMEOUT status! ********************\n");
            for (;;)
            {
#if 0
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }
#endif
                sem_wait(&g_semb_afc_main_proc);
                (g_struAfcCounter.HoldoverTimeoutCnt)++;
                if(AFC_OK !=  gps_check_clock())
                {
                    u16GpsAvailableCountInHoldver = 0;
                    if (1 == g_struAfcComp.LenthFlag)
                    {
                        u32compensatecount++;
                        if (u32compensatecount>=AFC_COMP_INTERVAL)
                        {
                            u32compensatecount = 0;
                            (g_struAfcComp.CompCnt)++;
                            afc_comp_func();
                        }
                    }
                }
                else
                {
                    u16GpsAvailableCountInHoldver++;

                    if (u16GpsAvailableCountInHoldver > 30)
                    {
                        u16GpsAvailableCountInHoldver = 30;
                        afc_dbg( "AFC_HOLDOVERTIMEOUT---Time Source is usable!\n");
                        u32GpsClkAvailableCount = 0;
                        for(u32i=0; u32i<5; u32i++)
                        {
                            sem_wait(&g_semb_afc_main_proc);
                            (g_struAfcCounter.HoldoverTimeoutCnt)++;
                            if(AFC_OK ==  gps_check_clock())
                            {
                                u32GpsClkAvailableCount++;
                            }
                            else
                            {
                                u32GpsClkAvailableCount = 0;
                                break;
                            }
                        }
                        if(u32GpsClkAvailableCount == AFC_HOLDOVER_GPS_AVAILABLE_TIMES_MIN)
                        {
                            u16LockTimes = 0;
                            u16unlockTimes = 0;
                            g_struAfcPhase.PhaseAdjFlag = PHASE_MODULATION_ENABLE;
                            for (;;)
                            {
                                sem_wait(&g_semb_afc_main_proc);
                                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                                {
                                    break;
                                }
                                (g_struAfcCounter.HoldoverTimeoutCnt)++;
                                (void)afc_phase_modulate();
                                if (AFC_OK ==  afc_check_locked() && AFC_OK ==  gps_check_clock())
                                {
                                    u16LockTimes++;
                                    u16unlockTimes = 0;
                                }
                                else
                                {
                                    u16LockTimes = 0;
                                    u16unlockTimes++;
                                    if (AFC_OK !=  gps_check_clock())
                                    {
                                        break;
                                    }
                                }
                                if (u16LockTimes > AFC_HOLDOVER_GPS_AVAILABLE_LOCK_TIMES_MIN)
                                {
                                    afc_dbg( "******************** AFC_HOLDOVER_TIMEOUT ---> AFC_LOCK ********************\n");
                                    sprintf(alarm_afc_status_change,"AFC status AFC_HOLDOVER_TIMEOUT change to AFC_LOCK");
                                    (void) afc_state_change(AFC_LOCK);
                                    g_struAfcFrq.Cnt20sHisLen = 1;
                                    (g_struAfcCounter.LockTimesCnt)++;
                                    g_struAfcCounter.HoldoverTimeoutCnt = 0;
                                    g_gps_lock = 1;
                                    break;
                                }
                                if (u16unlockTimes > AFC_HOLDOVER_NOT_LOCK_TIMES_MAX)
                                {
                                    afc_dbg( "******************** AFC_HOLDOVER ---> AFC_ABNORMAL ********************\n");
                                    (void) afc_state_change(AFC_ABNORMAL);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
#endif
        /*=============================AFC_ABNORMAL===========================================*/
        if(AFC_ABNORMAL == g_struAfcStat.PhaseLockStage)
        {
            afc_dbg( "Entered ABNORMAL status!\n");
            afc_log_out();//log output
#ifndef BSP_DEBUG
            VOS_AFCAbnormalRestartSystem();
#endif
            for(;;)
            {
                /*本地同步源，afc不做调整*/
                if((bts_synsrc_type != SYNSRC_GPS) && (bts_synsrc_type != SYNSRC_BD) && (bts_synsrc_type != SYNSRC_1PPS_TOD) && (bts_synsrc_type != SYNSRC_ES))
                {
                    break;
                }
                sleep(5);
                if(AFC_OK ==  gps_check_clock())
                {
                    if((AFC_OK != afc_check_ocxo())&&(g_afc_ocxo_flag==0))
                    {
                        sprintf(alarm_afc_ocxo,"AFC  OCXO uncontrol: CntDif1s_min = %d,CntDif1s_max = %d",CntDif1s_min,CntDif1s_max);
                        //bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_OCXO_UNCONTROL_ALARM,BSP_ALM_CLASS_CRITICAL,alarm_afc_ocxo,sizeof(alarm_afc_ocxo),0);
                        g_afc_ocxo_flag = 1;
                    }
                    else
                    {
                        if(g_afc_ocxo_flag==1)
                        {
                            sprintf(alarm_afc_ocxo,"AFC  OCXO uncontrol: CntDif1s_min = %d,CntDif1s_max = %d",CntDif1s_min,CntDif1s_max);
                            //bsp_interpro_clear_alarm(BSP_ALM_TYPE_CPU_PF,BSP_ALM_OCXO_UNCONTROL_ALARM,BSP_ALM_CLASS_CRITICAL,alarm_afc_ocxo,sizeof(alarm_afc_ocxo),0);
                            g_afc_ocxo_flag = 0;
                        }
                    }
                }

                if(AFC_PRINT_LEVEL_DEBUG < g_u32Afcprint_level)
                {
                    afc_dbg( "AFC  ABNORMAL.\n");
                }

            }
        }
    }
}

/*******************************************************************************
* 函数名称: t_ afc_get_freq_phase
* 功    能:
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32 t_afc_get_freq_phase(void)
{
    u32 count = 0;
    u32 log_count = 0;
    u32 u32i = 0;
    u32 u32j = 0;
    char  s8str2[200]= {0};
    char  s8str3[200]= {0};
    char  s8str4[200]= {0};
    s8  s8temp;

    while(1)
    {
        sem_wait(&g_semb_getphas_sendac);

        afc_syn_src();

        g_struAfcFrq.FrqAdjFlag = FREQUENCE_MODULATION_DISABLE;

        if (g_struAfcStat.PhaseLockStage > AFC_STARTUP)
        {
            phasedif_get(&(g_struAfcPhase.PhaseDif));
            afc_freq_count_1s(&(g_struAfcFrq.Cnt1s),&(g_struAfcFrq.CntDif1s));
            //printf("CPLD 1s 频率计数值 = %2d, CPLD 1s频率差 = %2d \n",g_struAfcFrq.Cnt1s,g_struAfcFrq.CntDif1s);
            if (g_u32afc_20sfrqcnt_start == 1)
            {
                afc_freq_modulate_prepare();
                if ( 1 == u32en20s)
                {
                    int i;
                    sprintf(s8str4,"first = %d, last = %d , 20s频率计数值 %d \n",s32Value20stotal[0],s32Value20stotal[20],g_struAfcFrq.Cnt20s);
                    memcpy(afc_log[log_count],s8str4,200);
                    for(i=0; i<21; i++)
                    {
                        sprintf(s8str4,"%d = %d \n",i,s32Value20stotal[i]);
                        memcpy(afc_log[log_count],s8str4,200);
                        sprintf(afc_log[log_count+1],"####################################################end###########################################################\n");
                        if(log_count++ > (AFC_LOG_LENTH-3))
                        {
                            log_count=0;
                        }
                    }
                }
            }
        }

        sem_post(&g_semb_afc_main_proc);

        if ((AFC_HOLDOVER == g_struAfcStat.PhaseLockStage) || (AFC_HOLDOVER_TIMEOUT == g_struAfcStat.PhaseLockStage) || (abs(g_struAfcPhase.PhaseDif) < PHASE_DIF_FILTER_VALUE))
        {
            g_struAfcPhase.PhaseDifAvg =  afc_phasedif_avg(NS_PHASE_DIF_LEN);
            //printf("g_struAfcPhase.PhaseDifAvg = %f\n",g_struAfcPhase.PhaseDifAvg);
        }

        if (g_struAfcStat.PhaseLockStage > AFC_WARMUP)
        {
            if(AFC_PRINT_LEVEL_NORMAL < g_u32Afcprint_level)
            {
                count++;
                if (count >= g_u32AfcPrintCnt)
                {
                    count = 0;
                    u32i++;

                    (void)sprintf(s8str2, "[%5d][state=%d]温度=%d  DAC=%4d  中心DAC=%4d  相位差---1s=%3d  30s=%4.1f  频率差---1s=%2d  %ds=%6.4f %d/%d  GPS = %d\n",
                                  u32i, g_struAfcStat.PhaseLockStage, g_struAfcComp.Temperature.s16TempValue[1], g_struAfcDac.DacValue,g_struAfcDac.DacLastTime,
                                  g_struAfcPhase.PhaseDif, g_struAfcPhase.PhaseDifAvg, g_struAfcFrq.CntDif1s, 20*g_struAfcFrq.Cnt20sHisLen, g_struAfcFrq.CntDifAvg,
                                  g_struAfcFrq.Cnt20sHisLen, g_struAfcFrq.Cnt20sHis,gps_check_clock()+1);

                    afc_dbg("%s",s8str2);
                }
            }

            sprintf(s8str3,"[%5d][state=%d]温度=%d  DAC=%4d  中心DAC=%4d  相位差---1s=%3d  30s=%4.1f  频率差---1s=%2d  %ds=%6.4f %d/%d  GPS = %d\n",
                    log_count, g_struAfcStat.PhaseLockStage, g_struAfcComp.Temperature.s16TempValue[1], g_struAfcDac.DacValue,g_struAfcDac.DacLastTime,
                    g_struAfcPhase.PhaseDif, g_struAfcPhase.PhaseDifAvg, g_struAfcFrq.CntDif1s, 20*g_struAfcFrq.Cnt20sHisLen, g_struAfcFrq.CntDifAvg,
                    g_struAfcFrq.Cnt20sHisLen, g_struAfcFrq.Cnt20sHis,gps_check_clock()+1);

            memcpy(afc_log[log_count],s8str3,200);
            sprintf(afc_log[log_count+1],"####################################################end###########################################################\n");
            if(log_count++ > (AFC_LOG_LENTH-3))
            {
                log_count=0;
            }
        }

        u32j++;
        if (4 <= u32j)
        {
            bsp_read_temp(0,4,&s8temp);
            g_struAfcComp.Temperature.s16TempValue[1] =(s16)s8temp;
            u32j = 0;
        }
    }
}

u32 get_afc_lock_times()
{
    return g_struAfcCounter.LockTimesCnt;
}

/*******************************************************************************
* 函数名称: AFC_CPLD_REG()
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
* 返回值:
* 无
* 说   明:
*******************************************************************************/
void AFC_CPLD_REG(void)
{
    u8 u8AheadHigh = 0;
    u8 u8AheadLow = 0;
    u16 u16AheadData;

    u8 u8LagHigh = 0;
    u8 u8LagLow = 0;
    u16 u16LagData;
    u16 u16TemHigh = 0;
    u16 u16TemLow = 0;

    u32 u32data = 0;
    s32 s32data = 0;

    u8AheadHigh = bsp_cpld_read_reg(CPLD_AFC_PD_AHEAD_CNT_1_REG);
    u8AheadLow = bsp_cpld_read_reg(CPLD_AFC_PD_AHEAD_CNT_0_REG);
    u16TemHigh = (((u16)u8AheadHigh)<<8) & ((u16)(0xFF00));
    u16TemLow = (u16)u8AheadLow;
    u16AheadData = (u16)(u16TemHigh | u16TemLow);

    afc_dbg( "超前相位 = %d\n", u16AheadData);

    u8LagHigh = bsp_cpld_read_reg(CPLD_AFC_PD_LAG_CNT_1_REG);
    u8LagLow = bsp_cpld_read_reg(CPLD_AFC_PD_LAG_CNT_0_REG);
    u16TemHigh = (((u16)u8LagHigh)<<8) & ((u16)(0xFF00));
    u16TemLow = (u16)u8LagLow;
    u16LagData = (u16)(u16TemHigh | u16TemLow);
    afc_dbg( "滞后相位 = %d\n", u16LagData);

    u32data = (bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_3_REG) << 24)
              |(bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_2_REG) << 16)
              |(bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_1_REG) << 8)
              |(bsp_cpld_read_reg(CPLD_AFC_FD_1S_CNT_0_REG));

    u32data &= 0x3ffffff;
    s32data = (s32)u32data -COUNTER_61M44;
    afc_dbg( "1s频率计数器 = %d\n", (s32)u32data);
    afc_dbg( "1s频率计数差 = %d\n", s32data);

    u32data = 0;
    u32data = (bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_3_REG) << 24)
              |(bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_2_REG) << 16)
              |(bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_1_REG) << 8)
              |(bsp_cpld_read_reg(CPLD_AFC_FD_20S_CNT_0_REG));

    u32data &= 0x7fffffff;

    afc_dbg( "20频率计数器 = %d\n", (s32)u32data);
    s32data = (s32)u32data - COUNTER_61M44*20;
    afc_dbg("20频率计数差 = %d\n", s32data);

}

/*******************************************************************************
* 函数名称: AFC_PRINT_VAR
* 功    能: 打印afc模块全局变量
* 相关文档:
* 函数类型: VOID
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值:  NONE
* 说   明:
*******************************************************************************/
void AFC_PRINT_VAR(void)
{
    switch (g_struAfcStat.PhaseLockStage)
    {
    case 0:
    {
        afc_dbg("AFC 状态---start\n");
        break;
    }

    case 1:
    {
        afc_dbg("AFC状态---warmup\n");
        break;
    }

    case 2:
    {
        afc_dbg("AFC状态---fast\n");
        break;
    }

    case 3:
    {
        afc_dbg("AFC状态---lock\n");
        break;
    }

    case 4:
    {
        afc_dbg("AFC状态---holdover\n");
        break;
    }

    case 5:
    {
        afc_dbg("AFC状态---holdover 超时\n");
        break;
    }

    case 6:
    {
        afc_dbg("AFC状态---abnormal\n");
        break;
    }

    default:
    {
        afc_dbg("AFC状态 --- %d\n",g_struAfcStat.PhaseLockStage);
    }
    }

    afc_dbg("调频长度n----%d\n",g_struAfcFrq.Cnt20sHisLen);
    afc_dbg("调频开关-----%d\n",g_struAfcFrq.FrqAdjFlag);
    afc_dbg("调相开关-----%d\n",g_struAfcPhase.PhaseAdjFlag);
    afc_dbg("Dac VALUE----%d\n",g_struAfcDac.DacValue);
    afc_dbg("PhaseDifAvg----%f\n",g_struAfcPhase.PhaseDifAvg);
    afc_dbg("\n");
    afc_dbg("上秒频率计数差 ----------  %d\n",g_struAfcFrq.CntDif1s);
    afc_dbg("60s频率平均计数差 -------  %f\n",g_struAfcFrq.CntDifAvg);
    afc_dbg("上秒相位计数差 ----------  %d\n",g_struAfcPhase.PhaseDif);
    afc_dbg("30s相位平均计数差 -------  %f\n",g_struAfcPhase.PhaseDifAvg);
    afc_dbg("\n");

    afc_dbg("g_struAfcComp.LenthFlag --------  %d\n",g_struAfcComp.LenthFlag);
    afc_dbg("g_struAfcComp.CalCnt -----------  %d\n", g_struAfcComp.CalCnt);
    afc_dbg("g_struAfcComp.CompCnt ----  %d\n", g_struAfcComp.CompCnt);
    afc_dbg("温度参数 = %f\n",g_struAfcComp.TemperatureCoef);
    afc_dbg("时间参数 = %f\n",g_struAfcComp.TimeCoef);
    afc_dbg("中心DAC  = %f\n",g_struAfcComp.DACCoef);
    afc_dbg("\n");
    afc_dbg("AFC Lock次数---%d\n",g_struAfcCounter.LockTimesCnt);
    afc_dbg("Holdover次数---%d\n",g_struAfcCounter.HoldoverTimesCnt);
    afc_dbg("Holdover时间---%d\n",g_struAfcCounter.HoldoverCnt);
    afc_dbg("HoldoverTimeout次数---%d\n",g_struAfcCounter.HoldoverTimeoutTimesCnt);
    afc_dbg("HoldoverTimeout时间---%d\n",g_struAfcCounter.HoldoverTimeoutCnt);
    afc_dbg("\n");

}

void bsp_get_afc_error_status(void)
{
    if(g_afc_ocxo_flag)
        printf("AFC OCXO uncontrol: CntDif1s_min = %d,CntDif1s_max = %d.\r\n",CntDif1s_min,CntDif1s_max);
    else
        printf("AFC OCXO ok!\r\n");
}


/******************************* 源文件结束 ***********************************/

