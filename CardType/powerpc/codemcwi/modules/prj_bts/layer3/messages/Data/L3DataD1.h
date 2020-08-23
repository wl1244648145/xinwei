/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataDelIL.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   11/01/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __SNOOP_DEL_ILENTRY_H__
#define __SNOOP_DEL_ILENTRY_H__

#include "Message.h"
#include "L3DataCommon.h"
#include "L3DataAssert.h"

/********************************
 *M_UT_MOVE_AWAY: UT漫游离开
 *M_UT_REMOVE   : UT被注销等
 ********************************/
#define M_UT_MOVE_AWAY     (0)
#define M_UT_REMOVE        (1)

/********************************
 *stDelIL: DM发起的删除Ip List
 *消息格式,由DM发往Snoop任务
 *不需要回应
 ********************************/
typedef struct _tag_stDelIL
{
    UINT8 aucMac[ M_MAC_ADDRLEN ];
    UINT8 ucOp;     /*取值 M_UT_MOVE_AWAY | M_UT_REMOVE */
} stDelIL;


/*****************************************
 *CDelILEntry类
 *****************************************/
class CDelILEntry:public CMessage
{
public:
    CDelILEntry(){}
    CDelILEntry(const CMessage &msg):CMessage( msg ){}
    ~CDelILEntry(){}

    void SetMac(const UINT8 *pMac)
        {
        if ( NULL == pMac )
            {
            DATA_assert( 0 );
            return;
            }
        memcpy( ( (stDelIL*)GetDataPtr() )->aucMac, pMac, M_MAC_ADDRLEN );
        }
    UINT8* GetMac() const   { return ( (stDelIL*)GetDataPtr() )->aucMac; }

    void   SetOp(UINT8 ucOp){ ( (stDelIL*)GetDataPtr() )->ucOp = ucOp; }
    UINT8  GetOp()          { return ( (stDelIL*)GetDataPtr() )->ucOp; }

protected:
    UINT32 GetDefaultDataLen() const{ return sizeof( stDelIL ); }
    UINT16 GetDefaultMsgId() const  { return MSGID_IPLIST_DELETE; }
};


/*****************************************
 *CclearCPEData类
 *通知snoop删除有关eid的所有数据
 *****************************************/
class CclearCPEData:public CMessage
{
public:
    CclearCPEData(){}
    //CclearCPEData(const CMessage &msg):CMessage( msg ){}
    ~CclearCPEData(){}

protected:
    UINT32 GetDefaultDataLen() const{ return 0; }
    UINT16 GetDefaultMsgId() const  { return MSGID_IPLIST_DELETE_BY_CPE; }
};


#endif /*__SNOOP_DEL_ILENTRY_H__*/

