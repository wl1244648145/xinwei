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

#ifndef _INC_L3EMSMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L3CPEETHCFGREQ
#include "L3L3CpeEthCfgReq.h"     
#endif

#include <string.h>
////////////////
//对于此消息要进行特殊处理,因为中间的数据是可变的,计算后面数据
//的偏移需要前面的数据,因此对数据的操作要按定义的顺序进行
////////////////

bool CL3CpeEthCfgReq :: CreateMessage(CComEntity &Entity, UINT32 uDataSize)
{
    if (false == CMessage :: CreateMessage(Entity, uDataSize))
        return false;

    //SetMessageId(M_L3_BTS_CPE_ETHERNET_CFG_REQ);

	return true;
}

UINT32 CL3CpeEthCfgReq :: GetDefaultDataLen() const
{
    return MAX_L3_CPE_ETH_CFG_REQ_MSG_LEN;
}

#if 0
UINT16 CL3CpeEthCfgReq :: GetTransactionId()const
{
    return ((T_L3CpeEthCfgReq*)GetDataPtr())->TransId;
}

UINT16 CL3CpeEthCfgReq :: SetTransactionId(UINT16 TransId)
{
    ((T_L3CpeEthCfgReq*)GetDataPtr())->TransId = TransId;
	return 0;
}
#endif

UINT16 CL3CpeEthCfgReq :: GetCmdType()const
{
    return ((T_L3CpeEthCfgReq*)GetDataPtr())->CmdType;
}

UINT16 CL3CpeEthCfgReq :: SetCmdType(UINT16 TransId)
{
   	((T_L3CpeEthCfgReq*)GetDataPtr())->CmdType = TransId;
	return 0;
}
UINT8 * CL3CpeEthCfgReq ::  GetCmdContent() const 
{
	return ((T_L3CpeEthCfgReq*)GetDataPtr())->Content;

}
//wangwenhua add 20081210 for debug


 bool CL3CpeCommMsgReq::CreateMessage(CComEntity  &Entity, UINT32 uDataSize)
{
      if (false == CMessage :: CreateMessage(Entity, uDataSize))
        return false;

     SetMessageId(M_L3_CPE_BTS_DEBUG_COMM_MSG);

	return true;

 }
 UINT32 CL3CpeCommMsgReq::GetDefaultDataLen() const
 {

      return sizeof(T_L3CpeCommMsgReq);
  }


UINT16   CL3CpeCommMsgReq::GetCommMsgType() const
{

     return ((T_L3CpeCommMsgReq*)GetDataPtr())->type;
}
UINT16   CL3CpeCommMsgReq::GetCommMsgLen() const
{
    return ((T_L3CpeCommMsgReq*)GetDataPtr())->Len;
}

UINT8 *CL3CpeCommMsgReq::GetCommContent()const 
{
  return  ((T_L3CpeCommMsgReq*)GetDataPtr())->Content;
}

bool CL3CpeCommCfgReq :: CreateMessage(CComEntity &Entity, UINT32 uDataSize)
 {
        if (false == CMessage :: CreateMessage(Entity, uDataSize))
        return false;

     SetMessageId(M_L3_CPE_BTS_COMM_MSG_CFG_REQ);

	return true;
 }
UINT32 CL3CpeCommCfgReq :: GetDefaultDataLen() const
{
    return sizeof(L3CpeCommCfgReq);
}

void    CL3CpeCommCfgReq :: SetCommMsgType(UINT16 type) 
{

	 ((T_L3CpeCommCfgReq*)GetDataPtr())->type = type;
}
//wangwenhua add end 20081211
