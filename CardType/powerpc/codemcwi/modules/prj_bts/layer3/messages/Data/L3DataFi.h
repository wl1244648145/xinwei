/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataAddFixIp.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/06/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_ADD_FIXIP_H__
#define __SNOOP_ADD_FIXIP_H__

#include "L3DataRoam.h"

/*****************************************
 *CADDFixIP类
 *结构完全和CRoam类一致，只是MsgId不一致
 *****************************************/
class CADDFixIP:public CRoam
{
public:
    CADDFixIP(){}
    CADDFixIP(const CMessage &msg):CRoam( msg ){}
    ~CADDFixIP(){}

protected:
    UINT16 GetDefaultMsgId() const { return MSGID_IPLIST_ADD_FIXIP; }
};


#endif/*__SNOOP_ADD_FIXIP_H__*/
