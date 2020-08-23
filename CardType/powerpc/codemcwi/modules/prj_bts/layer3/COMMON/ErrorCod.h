/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_ERRORCODEDEF
#define _INC_ERRORCODEDEF

#include "LogArea.h" 

#define  FMK_ERROR_DEBUG_INFO              LOGNO(FRMWK, 10)
#define  FMK_ERROR_INVALID_OBJECT          LOGNO(FRMWK, 11)
#define  FMK_ERROR_POST_MSG_FAIL           LOGNO(FRMWK, 12)
#define  FMK_ERROR_CREATE_MSG_QUEUE_FAIL   LOGNO(FRMWK, 13)
#define  FMK_ERROR_GET_MSG_FAIL            LOGNO(FRMWK, 14)
#define  FMK_ERROR_POST_NULL_MSG           LOGNO(FRMWK, 15)
#define  FMK_ERROR_NEW_COMMSG              LOGNO(FRMWK, 16)
#define  FMK_ERROR_DESTROY_NULL_MSG        LOGNO(FRMWK, 17)



#define  OS_ERROR_SUCCESS                   LOGNO(OS, 0)                        
#define  OS_ERROR_ERR_FAIL                  LOGNO(OS, 1)

#define  ERR_SUCCESS                        OS_ERROR_SUCCESS
#define  ERR_FAIL                           OS_ERROR_ERR_FAIL
/*
>reset Cause:
>00H － 硬件故障；
>01H － 软件故障
>02H － 传输链路故障
>03H － 断电
>04H － OMC要求复位
>05H — 系统升级
>06H — 不知道原因
>其他 － 目前保留。*/

//系统
#define  L3SYS_ERROR_BTS_RESET_HW          LOGNO(SYS, 0X0)
#define  L3SYS_ERROR_BTS_RESET_SW          LOGNO(SYS, 0X1)
#define  L3SYS_ERROR_BTS_RESET_LINK        LOGNO(SYS, 0X2)
#define  L3SYS_ERROR_BTS_RESET_POWEROFF    LOGNO(SYS, 0X3)
#define  L3SYS_ERROR_BTS_RESET_OMCREQ      LOGNO(SYS, 0X4)
#define  L3SYS_ERROR_BTS_RESET_CODE_UPDATE LOGNO(SYS, 0X5)
#define  L3SYS_ERROR_BTS_RESET_UNKNOWN     LOGNO(SYS, 0X6)
#define  L3SYS_ERROR_SYS_DATA_INIT_FAIL    LOGNO(SYS, 0X7)
#define  L3SYS_ERROR_APPCODE_LOAD_FAIL     LOGNO(SYS, 0X8)
#define  L3SYS_ERROR_REV_MSG               LOGNO(SYS, 0X10)      
#define  L3SYS_ERROR_REV_ERR_MSG           LOGNO(SYS, 0X11)   
#define  L3SYS_ERROR_RESET_BTS             LOGNO(SYS, 0X15)
#define  L3SYS_ERROR_RESET_L2PPC           LOGNO(SYS, 0X16)
#define  L3SYS_ERROR_RESET_AUX             LOGNO(SYS, 0X17)
#define  L3SYS_ERROR_RESET_MCP             LOGNO(SYS, 0X18)
#define  L3SYS_ERROR_RESET_FEP             LOGNO(SYS, 0X19)
#define  L3SYS_ERROR_BTS_BOOT_SUCCESS      LOGNO(SYS, 0X1A)
#define  L3SYS_ERROR_L2PPC_BOOT_SUCCESS    LOGNO(SYS, 0X1B)
#define  L3SYS_ERROR_AUX_BOOT_SUCCESS      LOGNO(SYS, 0X1C)
#define  L3SYS_ERROR_MCP_BOOT_SUCCESS      LOGNO(SYS, 0X1D)
#define  L3SYS_ERROR_FEP_BOOT_SUCCESS      LOGNO(SYS, 0X1E)
#define  L3SYS_ERROR_BTS_BOOT_FAIL         LOGNO(SYS, 0X20)
#define  L3SYS_ERROR_L2PPC_BOOT_FAIL       LOGNO(SYS, 0X21)
#define  L3SYS_ERROR_AUX_BOOT_FAIL         LOGNO(SYS, 0X22)
#define  L3SYS_ERROR_MCP_BOOT_FAIL         LOGNO(SYS, 0X23)
#define  L3SYS_ERROR_FEP_BOOT_FAIL         LOGNO(SYS, 0X24)
#define  L3SYS_ERROR_RESET_CPU             LOGNO(SYS, 0X25)
#define  L3SYS_ERROR_PRINT_SYS_INFO        LOGNO(SYS, 0XFF)

//配置
#define  L3CM_ERROR_REV_MSG                LOGNO(CM, 0X10)      
#define  L3CM_ERROR_REV_ERR_MSG            LOGNO(CM, 0X11)   
#define  L3CM_ERROR_CALIBRATION_ALM        LOGNO(CM, 0X12)   
#define  L3CM_ERROR_BTS_INIT_SUCCESS       LOGNO(CM, 0X13)   
#define  L3CM_ERROR_BTS_INIT_FAIL          LOGNO(CM, 0X14)
#define  L3CM_ERROR_PRINT_CFG_INFO         LOGNO(CM, 0XFF)
#define  L3CM_ERROR_TIMER_OUT              LOGNO(CM, 0XFFFF)
#ifdef WBBU_CODE
#define L3RRU_ERROR_REV_MSG                LOGNO(RRU, 0X10)  //wangwenhua add 20110228
#endif
//文件
#define  L3SM_ERROR_REV_MSG                LOGNO(SM, 0X10)      
#define  L3SM_ERROR_REV_ERR_MSG            LOGNO(SM, 0X11)      
#define  L3SM_ERROR_OPEN_FILE              LOGNO(SM, 0X12)      
#define  L3SM_ERROR_CREATE_FILE            LOGNO(SM, 0X13)      
#define  L3SM_ERROR_CLOSE_FILE             LOGNO(SM, 0X14)      
#define  L3SM_ERROR_SYS_STATE_DL           LOGNO(SM, 0X15)      
#define  L3SM_ERROR_MAX_CPE_NUM            LOGNO(SM, 0X16) //同时升级的CPE已经达到允许的最大个数
#define  L3SM_ERROR_CODE_FILE_ERR          LOGNO(SM, 0X17) //请求的代码文件同bts上的文件不一致
#define  L3SM_ERROR_FILE_NOT_EXIST         LOGNO(SM, 0X18) //请求消息中的文件在bts上不存在 
#define  L3SM_ERROR_CPE_UPDATING           LOGNO(SM, 0X19) //表示此CPE正在升级中，拒绝再次升级
#define  L3SM_ERROR_CPE_BC_UPDATING        LOGNO(SM, 0X1A) //表示此CPE正在升级中，拒绝再次升级
#define  L3SM_ERROR_CPE_HW_TYPE            LOGNO(SM, 0X1B) //表示升级命令中的类型不对
#define  L3SM_ERROR_CANT_LINKTO_SERVER     LOGNO(SM, 0X1C) //不能同ftp server建立连接
#define  L3SM_ERROR_USERNAME_PASSWORD      LOGNO(SM, 0X1D) //用户名或密码错
#define  L3SM_ERROR_SET_TRANS_TYPE         LOGNO(SM, 0X1E) //设置传输类型错
#define  L3SM_ERROR_GET_FILEDIR            LOGNO(SM, 0X1F) //获取文件目录错
#define  L3SM_ERROR_GET_FILENAEM           LOGNO(SM, 0X20) //获取文件名错
#define  L3SM_ERROR_INITP_DATASKT          LOGNO(SM, 0X21) //建立数据socket错
#define  L3SM_ERROR_GET_DATASKT            LOGNO(SM, 0X22) //建立数据socket错
#define  L3SM_ERROR_REV_DATA_SEQ_ERR       LOGNO(SM, 0X23) //CPE接收数据序号错
#define  L3SM_ERROR_ATA_DEVICE_ERR         LOGNO(SM, 0X24) //ATA not ready
#define  L3SM_ERROR_RETRY_TIMES            LOGNO(SM, 0X25) //发送次数超限
#define  L3SM_BTSDL_SUCCESS_1              LOGNO(SM, 0X26) //CPE接收数据序号错
#define  L3SM_BTSDL_SUCCESS_2              LOGNO(SM, 0X27) //ATA not ready
#define  L3SM_CPEDL_SUCCESS                LOGNO(SM, 0X28) //发送次数超限
#define  L3SM_ERROR_UPDATE_TIMEOUT   LOGNO(SM, 0X29) //下载结果消息超时jy081114
#define  L3SM_ERROR_BTS_DEAL_ERR  LOGNO(SM, 0X30) //基站处理发生异常
#define  L3SM_ERROR_PRINT_SM_INFO          LOGNO(SM, 0XFF)


//告警
#define  L3FM_ERROR_REV_MSG                LOGNO(FM, 0X10)      
#define  L3FM_ERROR_REV_ERR_MSG            LOGNO(FM, 0X11)      
#define  L3FM_ERROR_PRINT_SW_INFO          LOGNO(FM, 0XFF)


//终端
#define  L3UM_ERROR_REV_MSG                LOGNO(UM, 0X10)      
#define  L3UM_ERROR_REV_ERR_MSG            LOGNO(UM, 0X11)   
#define  L3UM_ERROR_CPE_INFO_PRINT         LOGNO(UM, 0XFF)


//定位
#define  L3GPS_ERROR_REV_MSG               LOGNO(GPS, 0X10)      
#define  L3GPS_ERROR_REV_ERR_MSG           LOGNO(GPS, 0X11)   
#define  L3GPS_ERROR_GPS_INFO_PRINT        LOGNO(GPS, 0XFF)    

//性能
#define  L3PM_ERROR_FTP_INFO_MSG           LOGNO(PM, 0X10) //FTP配置数据错误，IP＝0
#define  L3PM_ERROR_FTP_UPLOAD             LOGNO(PM, 0X11) //FTP文件上传错误，IP＝0
#define  L3PM_ERROR_WRITE_FILE             LOGNO(PM, 0X12) //写入性能文件错误，IP＝0

#ifdef WBBU_CODE
#define L3RRU_ERROR_REV_MSG LOGNO(RRU, 0x10)
#define L3RRU_ERROR_REV_ERR_MSG LOGNO(RRU, 0x11)
#define L3RRU_ERROR_RRU_UNREADY LOGNO(RRU, 0x30)
#endif
#endif
