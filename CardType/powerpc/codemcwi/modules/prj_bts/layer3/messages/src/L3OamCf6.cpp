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
#include "L3EmsMessageId.h"
#endif

// BTS Neighbor List Configuration Request（EMS）
#ifndef _INC_L3OAMCFGBTSNEIBLISTREQ
#include "L3OamCfgBtsNeibListReq.h" 
#endif
#include "ErrorCodeDef.h"
#include <string.h>

extern T_NvRamData *NvRamDataAddr;
CCfgBtsNeibListReq :: CCfgBtsNeibListReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgBtsNeibListReq :: CCfgBtsNeibListReq()
{
}

bool CCfgBtsNeibListReq :: CreateMessage(CComEntity &Entity, UINT32 datasize)
{
    CMessage :: CreateMessage(Entity, datasize);
    SetMessageId(M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_CFG_REQ);
    return true;
}

UINT32 CCfgBtsNeibListReq :: GetDefaultDataLen() const
{
    return sizeof(T_BtsNeighborCfgReq);
}

UINT16 CCfgBtsNeibListReq :: GetTransactionId() const
{
    return ((T_BtsNeighborCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgBtsNeibListReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsNeighborCfgReq *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CCfgBtsNeibListReq :: GetNeighborBtsNum() const
{
   UINT8 * pdata = (UINT8 *)GetDataPtr();

   return ((T_BtsNeighborCfgReq *)pdata)->NeighborBtsNum;
}

UINT32 CCfgBtsNeibListReq :: GetRealDataLen() const  ////后台配置消息的大小包括transid
{
    UINT16 NeigBtsNum = GetNeighborBtsNum() ;
    if(NeigBtsNum > NEIGHBOR_BTS_NUM)
    {
        return 2;
    }
    
    char *pdata = (char*)GetDataPtr() + 4;  //从第一个BtsNeighborCfgData开始计算
    UINT16 RepeaterNum = 0, repeaterLen = 0;
    UINT32 MsgLen = NeigBtsNum * (sizeof(T_BtsInfoIE)+sizeof(UINT32));
    for(int i = 0; i < NeigBtsNum; i++)
    {
         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
            return 2;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);
         MsgLen += repeaterLen;
         pdata += repeaterLen;       
    }

    MsgLen += 4;   
    return MsgLen ;
}

UINT32  CCfgBtsNeibListReq :: GetDataLenFromNvram()  //nvram中的真正配置数据的大小
{
    UINT16 NeigBtsNum = NvRamDataAddr->BtsNeighborCfgEle.NeighborBtsNum;
    if ((NeigBtsNum == 0) || (NeigBtsNum > NEIGHBOR_BTS_NUM))
    {
        return 2;
    }

    UINT16 RepeaterNum = 0, repeaterLen = 0;
    UINT32 MsgLen = NeigBtsNum * (sizeof(T_BtsInfoIE) + sizeof(UINT32));
    char *pdata = (char*)(NvRamDataAddr->BtsNeighborCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    for(int i = 0; i< NeigBtsNum; i++)
    {
         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
             return 2;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);
         MsgLen += repeaterLen;
         pdata += repeaterLen;       
    }

    MsgLen += 2;   
    return MsgLen ;
}

//DataBuff的前2字节 = Neighber list number.
void  CCfgBtsNeibListReq :: GetDataFromNvramForCpe(UINT8 *DataBuff, UINT32 &len)
{
    UINT16 NeigBtsNum = NvRamDataAddr->BtsNeighborCfgEle.NeighborBtsNum;
    len = 2;
    if((NeigBtsNum == 0) || (NeigBtsNum > NEIGHBOR_BTS_NUM))
    {
        *(UINT16*)DataBuff = 0;
        return;
    }

    UINT16 RepeaterNum = 0, repeaterLen = 0;
    char *pdata = (char*)(NvRamDataAddr->BtsNeighborCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算

    T_BtsInfoIE* pBtsInfoIE;
    T_BtsInfoIEForCPE BtsInfoIEForCPE;
    UINT16 i;
    UINT8 * pMsgData = DataBuff + sizeof(UINT16);
    UINT32 MsgForCpeLen = 0;
    for(i = 0; i< NeigBtsNum; i++)
    {
         ////////////////////////////////////////////////////
         pBtsInfoIE = &(((T_BtsNeighborCfgData*)pdata)->BtsInfoIE);
         BtsInfoIEForCPE.BTSID               = pBtsInfoIE->BTSID;
         BtsInfoIEForCPE.FrequencyIndex      = pBtsInfoIE->FrequencyIndex;
         BtsInfoIEForCPE.SequenceID          = pBtsInfoIE->SequenceID;
         BtsInfoIEForCPE.SubcarrierGroupMask = pBtsInfoIE->SubcarrierGroupMask;
         for(int j = 0; j<BCH_INFO_NUM; j++)
         {
         BtsInfoIEForCPE.BtsNeighborCfgChInfo.BCHInfo[j].BCHSCGIndex = pBtsInfoIE->BtsNeighborCfgChInfo.BCHInfo[j].BCHSCGIndex;
         BtsInfoIEForCPE.BtsNeighborCfgChInfo.BCHInfo[j].BCHTSIndex  = pBtsInfoIE->BtsNeighborCfgChInfo.BCHInfo[j].BCHTSIndex;
         }

         for(int k = 0; k<RRCH_INFO_NUM; k++)
         {
         BtsInfoIEForCPE.BtsNeighborCfgChInfo.RRCHInfor[k].RRCHSCGIndex = pBtsInfoIE->BtsNeighborCfgChInfo.RRCHInfor[k].RRCHSCGIndex;
         BtsInfoIEForCPE.BtsNeighborCfgChInfo.RRCHInfor[k].RRCHTSIndex  = pBtsInfoIE->BtsNeighborCfgChInfo.RRCHInfor[k].RRCHTSIndex;
         }

         for(int m = 0; m<RACH_RARCH_CFG_NUM; m++)
         {
         BtsInfoIEForCPE.BtsNeighborCfgChInfo.RACHPairInfor[m].RACHPairSCGIndex = pBtsInfoIE->BtsNeighborCfgChInfo.RACHPairInfor[m].RACHPairSCGIndex;
         BtsInfoIEForCPE.BtsNeighborCfgChInfo.RACHPairInfor[m].RACHPairTSIndex  = pBtsInfoIE->BtsNeighborCfgChInfo.RACHPairInfor[m].RACHPairTSIndex;
         }

         BtsInfoIEForCPE.N_ANT               = pBtsInfoIE->N_ANT;
         BtsInfoIEForCPE.TRANSMIT_PWR        = pBtsInfoIE->TRANSMIT_PWR;
         BtsInfoIEForCPE.N_TS                = pBtsInfoIE->N_TS;
         BtsInfoIEForCPE.N_DN_TS             = pBtsInfoIE->N_DN_TS;
         BtsInfoIEForCPE.RECEIVE_SENSITIVITY = pBtsInfoIE->RECEIVE_SENSITIVITY;
         BtsInfoIEForCPE.MAX_SCALE           = pBtsInfoIE->MAX_SCALE;
         BtsInfoIEForCPE.PREAMBLE_SCALE      = pBtsInfoIE->PREAMBLE_SCALE;
         BtsInfoIEForCPE.TCH_SCALE0          = pBtsInfoIE->TCH_SCALE0;
         BtsInfoIEForCPE.TCH_SCALE1          = pBtsInfoIE->TCH_SCALE1;
         BtsInfoIEForCPE.TCH_SCALE2          = pBtsInfoIE->TCH_SCALE2;
         BtsInfoIEForCPE.TCH_SCALE3          = pBtsInfoIE->TCH_SCALE3;
         BtsInfoIEForCPE.TCH_SCALE4          = pBtsInfoIE->TCH_SCALE4;
         BtsInfoIEForCPE.TCH_SCALE5          = pBtsInfoIE->TCH_SCALE5;
         BtsInfoIEForCPE.TCH_SCALE6          = pBtsInfoIE->TCH_SCALE6;
         memcpy(pMsgData, &BtsInfoIEForCPE, sizeof(T_BtsInfoIEForCPE));
         pMsgData += sizeof(T_BtsInfoIEForCPE);
         ///////////////////////////////////////////////////////

         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
             *(UINT16*)DataBuff = 0;
             return;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);

         ///////////////////////////////////////////////////////
         memcpy(pMsgData, pdata, repeaterLen);
         pMsgData += repeaterLen;
         ///////////////////////////////////////////////////////
         pdata += repeaterLen;
         UINT32 len = sizeof(T_BtsInfoIEForCPE) + repeaterLen;
         MsgForCpeLen += len;
         if(MsgForCpeLen > 3500) //超过最大消息长度
         {
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Neighbor BTS list length[%d] is almost reach the MAX (AI)length, stop copying to prevent ERRORs.", MsgForCpeLen);
            break;                 
         }
    }

    //MsgForCpeLen += 2;   
    //memcpy(DataBuff, &i, 2);
    len += MsgForCpeLen;
    *(UINT16*)DataBuff = i;
}


    
void  CCfgBtsNeibListReq :: GetNeiberBtsInfo(UINT16 *BtsNum, UINT32 *BtsIda)
{
    if(!BtsIda) return;
    if(!BtsNum) return;

    *BtsNum = NvRamDataAddr->BtsNeighborCfgEle.NeighborBtsNum;
    if(*BtsNum > NEIGHBOR_BTS_NUM)
    {
        return ;
    }
    
    UINT32 MsgLen = 0;   
    char *pdata = (char*)(NvRamDataAddr->BtsNeighborCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    UINT16 RepeaterNum = 0, repeaterLen= 0;
    for(int i = 0; i< *BtsNum; i++)
    {
         BtsIda[i] = ((T_BtsNeighborCfgData*)pdata)->BtsInfoIE.BTSID;
         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
             *BtsNum = RepeaterNum;
             return ;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);
         MsgLen += repeaterLen;
         pdata += repeaterLen;       
    }

}
CCfgBtsNeibListReq :: ~CCfgBtsNeibListReq()
{

}

CCfgBtsNeibListCommReq :: CCfgBtsNeibListCommReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgBtsNeibListCommReq :: CCfgBtsNeibListCommReq()
{
}

bool CCfgBtsNeibListCommReq :: CreateMessage(CComEntity &Entity, UINT32 datasize)
{
    CMessage :: CreateMessage(Entity, datasize);
    SetMessageId(M_EMS_BTS_NEIGHBOR_LIST_COMMON_CFG_REQ);
    return true;
}

UINT32 CCfgBtsNeibListCommReq :: GetDefaultDataLen() const
{
    return sizeof(T_BtsNeighborCommCfgReq);
}

UINT16 CCfgBtsNeibListCommReq :: GetTransactionId() const
{
    return ((T_BtsNeighborCommCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgBtsNeibListCommReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsNeighborCommCfgReq *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CCfgBtsNeibListCommReq :: GetNeighborBtsNum() const
{
   UINT8 * pdata = (UINT8 *)GetDataPtr();

   return ((T_BtsNeighborCommCfgReq *)pdata)->NeighborBtsNum;
}

UINT32 CCfgBtsNeibListCommReq :: GetRealDataLen() const  ////后台配置消息的大小包括transid
{
    UINT16 NeigBtsNum = GetNeighborBtsNum() ;
    if(NeigBtsNum > NEIGHBOR_BTS_NUM)
    {
        return 2;
    }
    
    char *pdata = (char*)GetDataPtr() + 4;  //从第一个BtsNeighborCfgData开始计算
    UINT16 RepeaterNum = 0, repeaterLen = 0;
    UINT32 MsgLen = NeigBtsNum * (sizeof(T_BtsInfoIE)+sizeof(UINT32));
    for(int i = 0; i < NeigBtsNum; i++)
    {
         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
            return 2;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);
         MsgLen += repeaterLen;
         pdata += repeaterLen;       
    }

    MsgLen += 4;   
    return MsgLen ;
}

UINT32  CCfgBtsNeibListCommReq :: GetDataLenFromNvram()  //nvram中的真正配置数据的大小
{
    UINT16 NeigBtsNum = NvRamDataAddr->BtsNeighborCommCfgEle.NeighborBtsNum;
    if ((NeigBtsNum == 0) || (NeigBtsNum > NEIGHBOR_BTS_NUM))
    {
        return 2;
    }

    UINT16 RepeaterNum = 0, repeaterLen = 0;
    UINT32 MsgLen = NeigBtsNum * (sizeof(T_BtsInfoIE) + sizeof(UINT32));
    char *pdata = (char*)(NvRamDataAddr->BtsNeighborCommCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    for(int i = 0; i< NeigBtsNum; i++)
    {
         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
             return 2;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);
         MsgLen += repeaterLen;
         pdata += repeaterLen;       
    }

    MsgLen += 2;   
    return MsgLen ;
}

    
void  CCfgBtsNeibListCommReq :: GetNeiberBtsInfo(UINT16 *BtsNum, UINT32 *BtsIda)
{
    if(!BtsIda) return;
    if(!BtsNum) return;

    *BtsNum = NvRamDataAddr->BtsNeighborCommCfgEle.NeighborBtsNum;
    if(*BtsNum > NEIGHBOR_BTS_NUM)
    {
        return ;
    }
    
    UINT32 MsgLen = 0;   
    char *pdata = (char*)(NvRamDataAddr->BtsNeighborCommCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    UINT16 RepeaterNum = 0, repeaterLen= 0;
    for(int i = 0; i< *BtsNum; i++)
    {
         BtsIda[i] = ((T_BtsNeighborCfgData*)pdata)->BtsInfoIE.BTSID;
         pdata  += sizeof(T_BtsInfoIE)+sizeof(UINT32);
         RepeaterNum = *(UINT16*)pdata;
         if(RepeaterNum > NEIGHBOR_BTS_NUM)
         {
             *BtsNum = RepeaterNum;
             return ;
         }
         repeaterLen = (RepeaterNum + 1) * sizeof(UINT16);
         MsgLen += repeaterLen;
         pdata += repeaterLen;       
    }

}
CCfgBtsNeibListCommReq :: ~CCfgBtsNeibListCommReq()
{

}
