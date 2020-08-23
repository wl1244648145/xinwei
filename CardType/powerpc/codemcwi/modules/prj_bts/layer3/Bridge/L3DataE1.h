/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEBBuffer.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   03/08/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __BRIDGE_BUFFER_H__
#define __BRIDGE_BUFFER_H__

#include "datatype.h"
#include "ComMessage.h"


class CBufferList
{
////Function members.
public:
    CBufferList():usNodeCount(0),head(NULL),rear(NULL){}
    ~CBufferList()
        {
        DeleteAll();
        }

    //缓存
    bool   Buffer(CComMessage*);
    //重新转发
    void   ReForward();
    //回收过期的buffer.
    void   Reclaim();

    UINT16 count() const    { return usNodeCount; }

private:
    void DeleteAll();
    void ForwardPacket(CComMessage*);

private:

/*********************
 *缓存链表结点结构
 *********************/
struct buffernode
{
////Data members.
    UINT32      ulTimeStamp;
    //UINT16      usBufLength;  //不new.保存完整的ComMessage.
    CComMessage *ptr2msgHdr;
    buffernode  *next; 

////Function members.
    buffernode():ulTimeStamp(0),ptr2msgHdr(NULL),next(NULL){}
    ~buffernode(){}
};

/***************************************
 *M_DEFAULT_BUFFER_LEASE: 缓存时长(tick)
 *M_MAGIC_BUFF_TIMESTAMP: 用于识别目的的时间戳
 */
#define M_DEFAULT_BUFFER_LEASE  (100)
#define M_MAGIC_BUFF_TIMESTAMP  (0x20060308)

////Data members.
    UINT16 usNodeCount;         //缓存的数据包个数
    struct buffernode *head;    //头指针
    struct buffernode *rear;    //尾指针
};

#endif/*__BRIDGE_BUFFER_H__*/
