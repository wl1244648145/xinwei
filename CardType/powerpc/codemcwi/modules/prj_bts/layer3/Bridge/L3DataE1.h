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

    //����
    bool   Buffer(CComMessage*);
    //����ת��
    void   ReForward();
    //���չ��ڵ�buffer.
    void   Reclaim();

    UINT16 count() const    { return usNodeCount; }

private:
    void DeleteAll();
    void ForwardPacket(CComMessage*);

private:

/*********************
 *����������ṹ
 *********************/
struct buffernode
{
////Data members.
    UINT32      ulTimeStamp;
    //UINT16      usBufLength;  //��new.����������ComMessage.
    CComMessage *ptr2msgHdr;
    buffernode  *next; 

////Function members.
    buffernode():ulTimeStamp(0),ptr2msgHdr(NULL),next(NULL){}
    ~buffernode(){}
};

/***************************************
 *M_DEFAULT_BUFFER_LEASE: ����ʱ��(tick)
 *M_MAGIC_BUFF_TIMESTAMP: ����ʶ��Ŀ�ĵ�ʱ���
 */
#define M_DEFAULT_BUFFER_LEASE  (100)
#define M_MAGIC_BUFF_TIMESTAMP  (0x20060308)

////Data members.
    UINT16 usNodeCount;         //��������ݰ�����
    struct buffernode *head;    //ͷָ��
    struct buffernode *rear;    //βָ��
};

#endif/*__BRIDGE_BUFFER_H__*/
