/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: FrameworkErrorCode.h
 *
 * DESCRIPTION:
 *     definition of error codes used by framework
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 02/18/2006  Yushu Shi   Initial file creation
 *---------------------------------------------------------------------------*/
#ifndef _FWK_ERROR_H
#define _FWK_ERROR_H

#include "LogArea.h" 

#define  FMK_ERROR_DEBUG_INFO              LOGNO(FRMWK, 0)
#define  FMK_ERROR_INVALID_OBJECT          LOGNO(FRMWK, 1)
#define  FMK_ERROR_POST_MSG_FAIL           LOGNO(FRMWK, 2)
#define  FMK_ERROR_CREATE_MSG_QUEUE_FAIL   LOGNO(FRMWK, 3)
#define  FMK_ERROR_GET_MSG_FAIL            LOGNO(FRMWK, 4)
#define  FMK_ERROR_POST_NULL_MSG           LOGNO(FRMWK, 5)
#define  FMK_ERROR_NEW_COMMSG              LOGNO(FRMWK, 6)
#define  FMK_ERROR_DESTROY_NULL_MSG        LOGNO(FRMWK, 7)

#endif //_FWK_ERROR_H
