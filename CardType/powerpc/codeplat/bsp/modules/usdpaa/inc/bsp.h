 
#ifndef BSP_H
#define BSP_H


/*---------------------------------------------------------------------------
�꿪���б�:
BSP_NOT_INCLUDE_SYS_HEADER: ����ҪBSP����vxworksͷ�ļ�������¶���

----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned long ulVirAddrStartH;
    unsigned long ulVirAddrStartL;
    unsigned long ulPhyAddrStartH;
    unsigned long ulPhyAddrStartL;
    unsigned long ulPageAttribute;
    unsigned short usPageSize;
    unsigned short usCfgFlag;
}T_Core_Mmu_Cfg_Table;
#ifdef __cplusplus
}
#endif


#endif /* BSP_H */
