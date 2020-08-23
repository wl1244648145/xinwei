/* =====================================================================
 *
 *  Copyright 2009-2011, Freescale Semiconductor, Inc., All Rights Reserved.
 *
 *  This file contains copyrighted material. Use of this file is restricted
 *  by the provisions of a Freescale Software License Agreement, which has
 *  either accompanied the delivery of this software in shrink wrap
 *  form or been expressly executed between the parties.
 *
 * ===================================================================*/
/******************************************************************************
 @File          fm_lib.c

 @Description   Frame Manager Linux User-Space library implementation.
*//***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "../inc/kernel_basic_define.h"

#include "../inc/fm_ioctls.h"
#include "../inc/fsl_pcd_api.h"
#include "../inc/fm_pcd_ioctls.h"


#define __ERR_MODULE__      MODULE_FM


/*******************************************************************************
*  FM FUNCTIONS                                                                *
*******************************************************************************/

t_Handle FM_Open(uint8_t id)
{
    t_Device    *p_Dev;
    int         fd;
    char        devName[20];

    p_Dev = (t_Device*) malloc(sizeof(t_Device));
    if (!p_Dev)
    {
        printf("FM_Open malloc error!!\n");
        return NULL;
    }

    memset(devName, 0, 20);
    sprintf(devName, "%s%s%d", "/dev/", DEV_FM_NAME, id);
    fd = open(devName, O_RDWR);
    if (fd < 0)
    {
       free(p_Dev);
       printf("Could not open FM. Ret code=%d, errno=%d. Aborting!!!\n",
               fd,
               errno);
       return NULL;
    }

    p_Dev->id = id;
    p_Dev->fd = fd;
    return (t_Handle)p_Dev;
}

void FM_Close(t_Handle h_Fm)
{
    t_Device    *p_Dev = (t_Device*) h_Fm;

    if (!h_Fm)
    {
        printf("FM_Close handle error!!\n");
    }

    close(p_Dev->fd);
    free(p_Dev);
}

/********************************************************************************************/
/*  FM_PCD FUNCTIONS                                                                        */
/********************************************************************************************/

t_Handle FM_PCD_Open(t_FmPcdParams *p_FmPcdParams)
{
    t_Device    *p_Dev;
    int         fd;
    char        devName[20];

    p_Dev = (t_Device*) malloc(sizeof(t_Device));
    if (!p_Dev)
    {
        printf("FM PCD device Open Error handle!!\n");
        return NULL;
    }

    memset(devName, 0, 20);
    sprintf(devName, "%s%s%d-pcd", "/dev/", DEV_FM_NAME, ((t_Device*) p_FmPcdParams->h_Fm)->id);
    fd = open(devName, O_RDWR);
    if (fd < 0)
    {
       free(p_Dev);
       printf("Could not open FM. Ret code=%d, errno=%d. Aborting!!!\n",
               fd,
               errno);
       return NULL;
    }

    p_Dev->id = ((t_Device*) p_FmPcdParams->h_Fm)->id;
    p_Dev->fd = fd;
    return (t_Handle) p_Dev;
}

void FM_PCD_Close(t_Handle h_FmPcd)
{
    t_Device    *p_Dev = (t_Device*) h_FmPcd;

    if (!h_FmPcd)
    {
        printf("FM PCD device Close error handle!!\n");
    }

    close(p_Dev->fd);
    free(p_Dev);
}

t_Error FM_PCD_CcNodeModifyKey(t_Handle h_FmPcd, t_Handle h_CcNode, uint8_t keyIndex, uint8_t keySize, uint8_t  *p_Key, uint8_t *p_Mask)
{
    t_Device    *p_Dev = (t_Device*) h_FmPcd;
    ioc_fm_pcd_cc_node_modify_key_params_t  params;
    uint8_t                                 key[IOC_FM_PCD_MAX_SIZE_OF_KEY];
    uint8_t                                 mask[IOC_FM_PCD_MAX_SIZE_OF_KEY];

    if (!h_FmPcd)
    {
        printf("FM_PCD_CcNodeModifyKey error h_FmPcd handle!!\n");
        return ~0x0;
    }

    params.id = h_CcNode;
    params.key_indx = keyIndex;
    params.key_size = keySize;
    memcpy(key, p_Key, keySize);
    if (p_Mask)
        memcpy(mask, p_Mask, keySize);
    params.p_key = key;
    params.p_mask = mask;

    if (ioctl(p_Dev->fd, FM_PCD_IOC_CC_NODE_MODIFY_KEY, &params))
    {
        printf("[usr debug] ioctl FM_PCD_IOC_CC_NODE_MODIFY_KEY p_Dev->fd:0x%x error\n", (unsigned int)p_Dev->fd);
        return (~0x0);
    }

    return 0;
}

//added by dongdeji for ltib v2.3.1
t_Handle FM_PCD_GetCcNodeInfo(t_Handle h_FmPcd, uint8_t ucFManDevId, uint8_t ucPortDevId, char *acDestName, uint8_t ucCcChainLevel, uint8_t *p_ucRouteIndexBase)
{
    t_Device    *p_Dev = (t_Device *)h_FmPcd;
    ioc_fm_CcNodeHandle_params_t  params;

    if (!h_FmPcd)
    {
        printf("FM_PCD_GetCcNodeInfo error h_FmPcd handle!!\n");
        return NULL;
    }
	
    memset(&params, 0x0, sizeof(ioc_fm_CcNodeHandle_params_t));
    params.ucFManDevId    = ucFManDevId;
    params.ucPortDevId    = ucPortDevId;
    params.ucCcChainLevel = ucCcChainLevel;
    memcpy(params.acDestName,acDestName,strlen(acDestName) + 1);
	
    printf("[debug] ucFManDevId:%u,ucPortDevId:%u,ucCcChainLevel:%u\n",ucFManDevId,ucPortDevId,ucCcChainLevel);
    if (ioctl(p_Dev->fd, FM_PCD_IOC_CC_NODE_GET_CC_NODE_INFO, &params))
    {
        printf("[usr debug] ioctl FM_PCD_IOC_CC_NODE_GET_CC_NODE_INFO p_Dev->fd:0x%x error\n", p_Dev->fd);
		*p_ucRouteIndexBase = 0xff;
        return NULL;
    }
	
    *p_ucRouteIndexBase = params.ucRouteIndexBase;
    return params.h_CcNode;
}



