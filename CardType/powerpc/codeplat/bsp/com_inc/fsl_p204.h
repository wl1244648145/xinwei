/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           fsl_p2041_ext.h 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
#ifndef BSP_P2041_EXT_H
#define BSP_P2041_EXT_H

Export  u8 *g_u8ccsbar;
Export  u8 *g_u8fpgabase;
Export  u8 *g_u8epldbase;

Export void bsp_sys_msdelay(ULONG dwTimeOut) ;
Export void bsp_sys_usdelay(ULONG dwTimeOut) ;
Export s32 dsp_download_status_get();

#endif
/******************************* ͷ�ļ����� ********************************/
