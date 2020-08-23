 
#ifndef BSP_H
#define BSP_H


/*---------------------------------------------------------------------------
宏开关列表:
BSP_NOT_INCLUDE_SYS_HEADER: 不需要BSP包含vxworks头文件的情况下定义

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
