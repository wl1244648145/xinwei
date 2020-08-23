/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataRoam.h
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

#ifndef __SNOOP_ROAM_H__
#define __SNOOP_ROAM_H__

#include "Message.h"
#include "L3DataMessages.h"


#pragma pack (1)
/*******************************************
 *stRoamReq:漫游请求的数据结构
 *******************************************/
typedef struct _tag_stRoamReq
{
    UINT32      ulEid;
    UTILEntry   Entry;
} stRoamReq;
#pragma pack ()


/*****************************************
 *CRoam类
 *****************************************/
class CRoam:public CMessage
{
public:
    CRoam(){}
    CRoam(const CMessage &msg):CMessage( msg ){}
    virtual ~CRoam(){}

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

    void SetEntry(const UTILEntry&);
    UTILEntry& GetEntry() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


#endif/*__SNOOP_ROAM_H__*/
