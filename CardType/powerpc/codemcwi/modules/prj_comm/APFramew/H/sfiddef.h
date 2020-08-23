/*****************************************************************************
               (C) Copyright 2005 : Arrowping Telecom
                   Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: sfiddef.h
 *
 * DESCRIPTION:
 *
 *    SFID(Service Flow ID) codes definitions.
 *    This code occupies a 16-bit word in every CComMessage
 *
 *
 * HISTORY:
 *
 *   Date        Author      Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/15/2005  LiuQun      Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_SFIDDEF
#define _INC_SFIDDEF

typedef enum
{
    M_SFID_INVALID = -1,
    M_SFID_HIGH = 0,
    M_SFID_LOW,
    M_SFID_REALTIME
}SFID;

#endif
