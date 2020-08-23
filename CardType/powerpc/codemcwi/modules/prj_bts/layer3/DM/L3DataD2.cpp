/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    DTSyncMsg.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   10/12/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>

#include "L3DataDmSyncMsg.h"



/*============================================================
MEMBER FUNCTION:
    CDtACLUpdtReq

DESCRIPTION:
   Define functions of class  CDtACLUpdtReq
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
//
UINT32 CDtACLUpdtReq::GetDefaultDataLen() const
{
    return(sizeof(ACLDldReq));
}

UINT16 CDtACLUpdtReq::GetDefaultMsgId() const
{
    return(MSGID_UT_BTS_PROTOCOLFLTR_TABLEUPDATE_REQ);
}
UINT16    CDtACLUpdtReq::SetACLXid(UINT16 id)
{
    return((ACLDldReq*)GetDataPtr())->Xid =htons(id);
}
UINT16    CDtACLUpdtReq::SetACLVersion(UINT16 ver)
{
    return((ACLDldReq*)GetDataPtr())->Version =htons(ver);
}
UINT32    CDtACLUpdtReq::SetACLNum(UINT32 ucnm)
{
    return((ACLDldReq*)GetDataPtr())->ucNo =htonl(ucnm);
}


void CDtACLUpdtReq::SetValue(const PrtclFltrEntry *PrtlFltTb,const UINT32 ucno)
{
    PrtclFltrEntry *ptmp =(PrtclFltrEntry*)( ((ACLDldReq*)GetDataPtr())->arACLTb);
    for ( UINT32 i=0;i<ucno;i++ )
        {
        ptmp[i].ucOccupied = PrtlFltTb[i].ucOccupied;
        ptmp[i].ucFlag = PrtlFltTb[i].ucFlag;
        ptmp[i].ucIsBroadcast = PrtlFltTb[i].ucIsBroadcast;
        ptmp[i].usProtocol = htons(PrtlFltTb[i].usProtocol);
        ptmp[i].ulSrcAddr = htonl(PrtlFltTb[i].ulSrcAddr);
        ptmp[i].ulSrcMask = htonl(PrtlFltTb[i].ulSrcMask);
        ptmp[i].ucSrcOp = PrtlFltTb[i].ucSrcOp;
        ptmp[i].usSrcPort = htons(PrtlFltTb[i].usSrcPort);
        ptmp[i].ulDstAddr = htonl(PrtlFltTb[i].ulDstAddr);
        ptmp[i].ulDstMask = htonl(PrtlFltTb[i].ulDstMask);
        ptmp[i].ucDstOp = PrtlFltTb[i].ucDstOp;
        ptmp[i].usDstPort = htons(PrtlFltTb[i].usDstPort);
        ptmp[i].ucPermit = PrtlFltTb[i].ucPermit;
        }
}

/*============================================================
MEMBER FUNCTION:
    CDtDataServsRsp

DESCRIPTION:
   Define functions of class  CDtDataServsRsp
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
UINT32 CDtDataServsRsp::GetDefaultDataLen() const
{
    return(sizeof(DataServsRsp));
}

UINT16 CDtDataServsRsp::GetDefaultMsgId() const
{
    return(MSGID_UT_BTS_DATASERVICE_RESP);
}

UINT8 CDtDataServsRsp::SetResult(bool rst)
{
    return( (DataServsRsp*)GetDataPtr() )->Result =rst?(1):(0); 
}

UINT8 CDtDataServsRsp::SetWorkMode(UINT8 wrkMd)
{
    return( (DataServsRsp*)GetDataPtr() )->WorkMode = wrkMd;
}

UINT16 CDtDataServsRsp::SetVlanID(UINT16 usVlanID)
{
    return( (DataServsRsp*)GetDataPtr() )->usVlanID = htons(usVlanID);
}


/*============================================================
MEMBER FUNCTION:
    CDtSyncIplistReq

DESCRIPTION:
   Define functions of class  CDtSyncIplistReq
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/

UINT32 CDtSyncIplistReq::GetDefaultDataLen() const
{
    return(sizeof(DAIBReq));
}

UINT16 CDtSyncIplistReq::GetDefaultMsgId() const
{
    return(MSGID_UT_BTS_DAIBUPDATE_REQ);
}

void CDtSyncIplistReq::SetXid(UINT16 id)
{
    ( (DAIBReq*)GetDataPtr() )->Xid=htons(id) ;
}
void CDtSyncIplistReq::SetTimeStamp(UINT32 timeSt)
{
    ( (DAIBReq*)GetDataPtr() )->TimeStamp=htonl(timeSt) ;
}
void CDtSyncIplistReq::SetVersion(UINT16 ver)
{
    ( (DAIBReq*)GetDataPtr() )->Version=htons(ver) ;
}
void CDtSyncIplistReq::SetIpCount(UINT32 num)
{
    ( (DAIBReq*)GetDataPtr() )->IpCount=htonl(num);
}

void  CDtSyncIplistReq::Set_DAIBTLV(const UTILEntry* Iplst,UINT32 ipNm)
{
    // assert(NULL!=Iplst);
    IplistTLV *pBuf= ( (DAIBReq*)GetDataPtr() )->IplstTLV;
    for ( UINT8 i=0;i<ipNm;i++ )
        {
        if ( IPTYPE_FIXIP == Iplst[i].ucIpType )
            {
            pBuf[i].Type = FIXIPLIST; 
            }
        else
            {
            pBuf[i].Type = DYNIPLIST;
            }
        pBuf[i].Len =(UINT16)( sizeof(UTILEntry) );

        memcpy(&(pBuf[i].Iplist),&Iplst[i],sizeof(UTILEntry));
        }
}


/*
Define  CDtSyncAddrFBReq
*/

UINT32 CDtSyncAddrFBReq::GetDefaultDataLen() const
{
    return(sizeof(AddrFltTbReq));
}
UINT16 CDtSyncAddrFBReq::GetDefaultMsgId() const
{
    return(MSGID_UT_BTS_ADDRFLTR_TABLEUPDATE_REQ);
}
UINT16 CDtSyncAddrFBReq::SetAddrXid(UINT16 id)
{
    return( (AddrFltTbReq*)GetDataPtr() )->Xid=htons(id);
}
UINT16 CDtSyncAddrFBReq::SetAddrVersion(UINT16 ver)
{
    return( (AddrFltTbReq*)GetDataPtr() )->Version=htons(ver);
}

UINT32 CDtSyncAddrFBReq::SetAddrCount(UINT32 ucNm)
{
    return( (AddrFltTbReq*)GetDataPtr() )->addrCount=htonl(ucNm);
}
void CDtSyncAddrFBReq::SetAddrFB(const AddressFltrTb* addrFlt,const UINT32 ucNm)
{
    //assert(NULL!=addrFlt);
    AddressFltrTb* addrEntry=( (AddrFltTbReq*)GetDataPtr() )->AddrsFltrTb;
    //memcpy(addrEntry,addrFlt,ucNm*sizeof(AddressFltrTb));
    for ( UINT8 i=0;i<ucNm;i++ )
        {
        memcpy(addrEntry[i].ucSrcMAC,addrFlt[i].ucSrcMAC,M_MAC_ADDRLEN);
        memcpy(addrEntry[i].ucPeerMAC,addrFlt[i].ucPeerMAC,M_MAC_ADDRLEN);
        addrEntry[i].usSessionId = htonl(addrFlt[i].usSessionId);
        addrEntry[i].ulSrcIpAddr = htonl(addrFlt[i].ulSrcIpAddr);
        }
}

