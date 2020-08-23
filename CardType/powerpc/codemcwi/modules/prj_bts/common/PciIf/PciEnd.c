/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*******************************************************************
*
* FILENAME: PciEnd.cpp
* DESCRIPTION:
*   This file contains the END driver over PCI
*
*
* HISTORY:
*
*   Date       Author  Description
*   ---------  ---------  -------------------------------------------
*   1/6/2006   Yushu Shi Initial file creation.
*
*******************************************************************/


#include <vxWorks.h>
#include <endLib.h>
#include <etherLib.h>
#include <muxLib.h>
#include <netLib.h>
#include <netBufLib.h>
#include <net/if.h>
#include <net/route.h>
#include <taskLib.h>
#include <usrConfig.h>
#include <stdio.h>

#include "mcWill_bts.h"

typedef STATUS (*PCISENDFUNC) (M_BLK_ID);		/* pfunction returning int */


PCISENDFUNC PciEndSendFunc = NULL; /* implemented in pciIf.cpp */

extern int ipAttach(int unit, char* pDevice);

#define ENET_SIZE             6       /* Ethernet address size */

#define END_HADDR(pEnd) ((pEnd)->pMib2Tbl->m2Data.mibIfTbl.ifPhysAddress.phyAddress)
#define END_HADDR_LEN(pEnd) ((pEnd)->pMib2Tbl->m2Data.mibIfTbl.ifPhysAddress.addrLength)

/* Global Variables*/
/*#define DEBUG_PCI_END */
BOOL DebugPciEnd = FALSE;
#ifdef DEBUG_PCI_END
#define DRV_LOG(X0)     printf(X0)
#define DRV_LOG_ARG(X0, X1)    printf(X0,X1)
#else
#define DRV_LOG(X0)   ;
#define DRV_LOG_ARG(X0,X1) ;
#endif
char devNamePrefix[8] = "PciEnd";
END_OBJ*     PciEndDrv = NULL;

STATUS    PciEndStart(END_OBJ* pEnd);
STATUS    PciEndSend (END_OBJ *, M_BLK_ID);
int       PciEndIoctl(END_OBJ* pEnd, int cmd, caddr_t data);
STATUS    PciEndMCastAddrAdd (END_OBJ *, char *);
STATUS    PciEndMCastAddrDel (END_OBJ *, char *);
STATUS    PciEndMCastAddrGet (END_OBJ *, MULTI_TABLE *);
STATUS    PciEndStop (END_OBJ *);
STATUS    PciEndUnload(END_OBJ *);
STATUS    PciEndPollSend (END_OBJ *, M_BLK_ID);
STATUS    PciEndPollRcv (END_OBJ *, M_BLK_ID);
STATUS    PciEndmCastAddrAdd(END_OBJ* pEnd, char* addr);
STATUS    PciEndmCastAddrDel(END_OBJ* pEnd, char* addr);
STATUS    PciEndmCastAddrGet(END_OBJ* pEnd, MULTI_TABLE* tbl);

NET_FUNCS PciEndFuncTbl = 
{
    PciEndStart,
    PciEndStop,
    PciEndUnload,
    PciEndIoctl,
    PciEndSend,
    PciEndmCastAddrAdd,
    PciEndmCastAddrDel,
    PciEndmCastAddrGet,
    PciEndPollSend,
    PciEndPollRcv,
    endEtherAddressForm,                /* put address info into a NET_BUFFER*/
    endEtherPacketDataGet,              /* get pointer to data in NET_BUFFER*/
    endEtherPacketAddrGet               /* Get packet addresses. */
};

/*******************************************************************************
*
* PciEndLoad - initialize the driver and device
*
* This routine initializes the driver and the device to the operational state.
* All of the device specific parameters are passed in the initString.
*
* The string contains the target specific parameters like this:
*
* "register addr:int vector:int level:shmem addr:shmem size:shmem width"
*
* RETURNS: An END object pointer or NULL on error.
*/
END_OBJ* PciEndLoad
(
    char* initString,
    void* privateData
)
{
    END_OBJ *pEnd;
    STATUS rtv;
    UINT8 addr[6] = {0x00,0x11,0x22,0x33,0x44,0x55};

    DRV_LOG("PciEndLoad\n");

    if(initString == NULL)
    {
        DRV_LOG("Invalid argument for Load\n");
        return(NULL);
    }

    if(initString[0] == 0)
    {
        DRV_LOG("Getting Driver name...\n");
        memcpy(initString, devNamePrefix, strlen(devNamePrefix));
        return NULL;
    }

    DRV_LOG ("Loading PCI End...\n");

    /* Parse the initialization string */


    pEnd = (END_OBJ*)calloc(sizeof(END_OBJ),1);
    if(NULL == pEnd)
    {
        DRV_LOG("PciEndLoad Cannot allocate END Device control block\n");
        return(NULL);
    }

    pEnd->pFuncTable = &PciEndFuncTbl;

    strcpy(pEnd->devObject.name, devNamePrefix);
    pEnd->devObject.unit = 0;
    strcpy(pEnd->devObject.description, "PCI END Driver.");

    pEnd->attached = FALSE;
    pEnd->flags    = 0;
    pEnd->txSem    = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);

    /* Initialize END object data structures */
    if(END_OBJ_INIT(pEnd, (DEV_OBJ*)&pEnd->devObject, devNamePrefix, 0, &PciEndFuncTbl, 
                    "PCI END Driver") == ERROR)
    {
        DRV_LOG("end obj_init failed\n");
        free ((char *)pEnd);
        return NULL;
    }
    
    #ifdef M_TGT_L2
    addr[5] +=1;
    #endif;

    rtv = END_MIB_INIT (pEnd,
                        M2_ifType_ethernet_csmacd,
                        addr,
                        ENET_SIZE,
                        ETHERMTU,
                        20000000);  /* 20Mbps */

	END_OBJ_READY (pEnd,
	               IFF_NOTRAILERS | IFF_MULTICAST | IFF_BROADCAST);

    if (endFindByName ( initString, 0) != NULL)
    {
        DRV_LOG_ARG("can not find pciend %s\n", initString);
    }

    if (endFindByName ( initString, 1) != NULL)
    {
        DRV_LOG_ARG("can not find pciend %s\n", initString);
    }
    DRV_LOG("endof pciendload\n");

    PciEndDrv = pEnd;
    return pEnd;
}


STATUS PciEndStart(END_OBJ* pEnd)
{
    DRV_LOG("PciEndStart\n");

    if(NULL == pEnd)
    {
        DRV_LOG("Bad END Obj\n");
        return ERROR;
    }

	END_FLAGS_SET(pEnd, (IFF_UP | IFF_RUNNING));

    return OK;
}

STATUS PciEndStop(END_OBJ* pEnd)
{
    DRV_LOG("PciEndStop\n");

    if(NULL == pEnd)
    {
        DRV_LOG("Bad END Obj\n");
        return ERROR;
    }

    return OK;
}

STATUS PciEndUnload(END_OBJ* pEnd)
{
    DRV_LOG("PciEndUnload\n");

    if(NULL == pEnd)
    {
        DRV_LOG("Bad END Obj\n");
        return ERROR;
    }

    return OK;
}

int PciEndIoctl(END_OBJ* pEnd, int cmd, caddr_t data)
{
    int value;

    DRV_LOG_ARG("PciEndIoctl - %08x ", cmd);

    if(NULL == pEnd)
    {
        DRV_LOG("\nBad END Obj\n");
        return EINVAL;
    }

    switch(cmd)
    {
    case EIOCSADDR:
        DRV_LOG("EIOCSADDR\n");
        if(data == NULL)
        {
            return(EINVAL);
        }
        bcopy ((char *)data, (char *)END_HADDR(pEnd), END_HADDR_LEN(pEnd));
        break;

    case EIOCGADDR:
        DRV_LOG("EIOCGADDR\n");
        if(data == NULL)
        {
            return(EINVAL);
        }
        bcopy ((char *)END_HADDR(pEnd), (char *)data, END_HADDR_LEN(pEnd));
        break;

    case EIOCSFLAGS:
        DRV_LOG_ARG("EIOCSFLAGS %08x\n", (int)data);


        value = (int)data;
        if(value < 0)
        {
            value = -(--value);
            END_FLAGS_CLR (pEnd, value);
        }
        else
        {
            END_FLAGS_SET (pEnd, value);
        }
        break;

    case EIOCGFLAGS:
        DRV_LOG("EIOCGFLAGS\n");
        *(int *)data = END_FLAGS_GET(pEnd);
        break;

    case EIOCPOLLSTART: /* Begin polled operation */
        DRV_LOG("EIOCPOLLSTART\n");
        return(EINVAL);
        break;

    case EIOCPOLLSTOP:  /* End polled operation */
        DRV_LOG("EIOCPOLLSTOP\n");
        return(EINVAL);
        break;

    case EIOCGMIB2:     /* return MIB information */
        DRV_LOG("EIOCGMIB2\n");
        if(data == NULL)
        {
            return(EINVAL);
        }
        bcopy((char *)&pEnd->mib2Tbl, (char *)data, sizeof(pEnd->mib2Tbl));
        break;

    case EIOCGFBUF: /* return minimum First Buffer for chaining */
        DRV_LOG("EIOCGFBUF\n");
        if(data == NULL)
        {
            return(EINVAL);
        }
        *(int *)data = 1536;
        break;

    case EIOCGHDRLEN:
        DRV_LOG("EIOCGHDRLEN\n");
        return EINVAL;
        break;

    case EIOCGMWIDTH:
        DRV_LOG("EIOCGMWIDTH\n");
        return EINVAL;
        break;

    case EIOCQUERY:
        DRV_LOG("EIOCQUERY\n");
        return EINVAL;
        break;
    case EIOCGNPT:
        DRV_LOG("EIOCGNPT\n");
        return EINVAL;
        break;
    case EIOCGMIB2233:
        DRV_LOG("EIOCGMIB2233\n");
        if ((data == NULL) || (pEnd->pMib2Tbl == NULL))
        {
            return EINVAL;
        }
        else
        {
            *((M2_ID **)data) = pEnd->pMib2Tbl;
        }

        break;

    case EIOCMULTIADD:
        DRV_LOG("EIOCMULTIADD\n");
        return EINVAL;
        break;
    case EIOCMULTIDEL:
        DRV_LOG("EIOCMULTIDEL\n");
        return EINVAL;
        break;
    case EIOCMULTIGET:
       DRV_LOG("EIOCMULTIGET\n");
       return EINVAL;
       break;
    default:
        DRV_LOG("UNKNOWN\n");
        return EINVAL;
    }

    return OK;
}

void DumpMbuf(M_BLK_ID pMblk)
{
    int i;

    if (DebugPciEnd)
    {
        M_BLK_ID  pCurr = pMblk;

        int fragNum = 0;
        int pktSize = 0;
        while (pCurr != NULL)
        {
            fragNum++;
            pktSize += pCurr->mBlkHdr.mLen;

            printf("Frag No%d:len=BLK%d:PH%d:AGG%d:T%02x:F%02x:D%08x:RSVD%08x\n", 
                   fragNum, pCurr->mBlkHdr.mLen, 
                   pCurr->mBlkPktHdr.len, pktSize,
                   pCurr->mBlkHdr.mType, pCurr->mBlkHdr.mFlags, 
                   (int)pCurr->mBlkHdr.mData, pCurr->mBlkHdr.reserved);

            if(NULL != pCurr->mBlkHdr.mData)
            {
                for(i = 0; i < pCurr->mBlkHdr.mLen; i++)
                {
                    printf("%02x ", pCurr->mBlkHdr.mData[i]);
                }
                printf("\n");
            }

            pCurr = pCurr->mBlkHdr.mNext;
        }

        printf("------------------------------------------------------------------------------\n");
    }
}


STATUS PciEndSend(END_OBJ* pEnd, M_BLK_ID pMblk)
{
    DRV_LOG("PciEndSend\n");

    if(NULL == pMblk)
    {
        DRV_LOG("Bad MBLK\n");
        return ERROR;
    }

    DumpMbuf(pMblk);

    if (PciEndSendFunc)
    {
        return PciEndSendFunc(pMblk);
    }
    else
    {
        DRV_LOG("Send IP packet through PCI END before ready\n");
        return ERROR;
    }

}


STATUS PciEndmCastAddrAdd(END_OBJ* pEnd, char* addr)
{
    DRV_LOG("PciEndmCastAddrAdd\n");

    return ERROR;
}

STATUS PciEndmCastAddrDel(END_OBJ* pEnd, char* addr)
{
    DRV_LOG("PciEndmCastAddrDel\n");

    return ERROR;
}

STATUS PciEndmCastAddrGet(END_OBJ* pEnd, MULTI_TABLE* tbl)
{
    DRV_LOG("PciEndmCastAddrGet\n");

    return ERROR;
}

STATUS PciEndPollSend(END_OBJ* pEnd, M_BLK_ID pMblk)
{
    DRV_LOG("PciEndPollSend\n");

    return ERROR;
}

STATUS PciEndPollRcv(END_OBJ* pEnd, M_BLK_ID pMblk)
{
    DRV_LOG("PciEndPollRcv\n");

    return ERROR;
}

M_BLK_ID PciEndFormAddress(M_BLK_ID pMblk, M_BLK_ID pMblkSrc, M_BLK_ID pMblkDst, int tbd)
{
    DRV_LOG("PciEndFormAddress\n");

    if(NULL == pMblk)
    {
        DRV_LOG("Bad pBlkRaw\n");
        return NULL;
    }

    if(NULL == pMblkSrc)
    {
        DRV_LOG("Bad pMblkSrc\n");
        return NULL;
    }

    if(NULL == pMblkDst)
    {
        DRV_LOG("Bad pMblkDst\n");
        return NULL;
    }

    if(pMblkSrc->mBlkHdr.reserved != pMblkDst->mBlkHdr.reserved)
    {
        DRV_LOG("Non matching dst/src addr prot types\n");
        return NULL;
    }

    M_PREPEND(pMblk, SIZEOF_ETHERHEADER, M_WAIT);

    memcpy(&pMblk->mBlkHdr.mData[0],  &pMblkDst->mBlkHdr.mData[0], 6);
    memcpy(&pMblk->mBlkHdr.mData[6],  &pMblkSrc->mBlkHdr.mData[0], 6);
    memcpy(&pMblk->mBlkHdr.mData[12], &pMblkDst->mBlkHdr.reserved, 2);

    if(  (0xff == pMblk->mBlkHdr.mData[0])
      && (0xff == pMblk->mBlkHdr.mData[1])
      && (0xff == pMblk->mBlkHdr.mData[2])
      && (0xff == pMblk->mBlkHdr.mData[3])
      && (0xff == pMblk->mBlkHdr.mData[4])
      && (0xff == pMblk->mBlkHdr.mData[5])
      )
    {
        pMblk->mBlkHdr.mFlags |= M_BCAST;
    }
    else if (0x01 == pMblk->mBlkHdr.mData[0] & 0x01)
    {
        pMblk->mBlkHdr.mFlags |= M_MCAST;
    }

    if(NULL != pMblk->mBlkHdr.mNextPkt)
    {
        DRV_LOG("Pkt chaining not supported\n");
        netMblkClChainFree(pMblk);
        return NULL;
    }

    if(ETHERTYPE_IP != pMblkDst->mBlkHdr.reserved)
    {
        DRV_LOG("Dropping Non-IP dgram\n");
        netMblkClChainFree(pMblk);
        return NULL;
    }
    
    return pMblk;
}

STATUS PciEndPacketDataGet(M_BLK_ID pMblk, LL_HDR_INFO* llHdr)
{
    DRV_LOG("PciEndPacketDataGet\n");

    if(NULL == pMblk)
    {
        DRV_LOG("Bad MBLK\n");
        return ERROR;
    }

    if(NULL == llHdr)
    {
        DRV_LOG("Bad llHdr\n");
        return ERROR;
    }

    if(pMblk->mBlkHdr.mLen < SIZEOF_ETHERHEADER)
    {
        DRV_LOG("Frm too short\n");
        return ERROR;
    }

    llHdr->destAddrOffset  = 0;
    llHdr->destSize        = 6;
    llHdr->srcAddrOffset   = 6;
    llHdr->srcSize         = 6;
    llHdr->ctrlAddrOffset  = 1;
    llHdr->ctrlSize        = pMblk->mBlkPktHdr.len; 
    llHdr->pktType         = (pMblk->mBlkHdr.mData[12] << 8) | pMblk->mBlkHdr.mData[13];
    llHdr->dataOffset      = SIZEOF_ETHERHEADER;

    return OK;
}


char   myNetworkStr  [BOOT_TARGET_ADDR_LEN];
#ifdef M_TGT_L3
char PeerName[8] = "L2PPC";
#else
char PeerName[8] = "L3PPC";
#endif
char PeerInetStr[INET_ADDR_LEN];


STATUS StartPciEnd(NET_POOL_ID pNetPool)
{      
    void * pCookie; 
    END_OBJ *myEnd ;
    char ifname[20];
    struct in_addr myInet, peerInet,networkInet;
    BOOT_PARAMS  params;
    char   networkStr[20];

    strcpy(ifname, devNamePrefix);

    DRV_LOG("PciEndDevStart()\n");
    myEnd = endFindByName(devNamePrefix,0);

    myEnd->pNetPool = pNetPool;

    DRV_LOG("PciEndipAttach()\n");
    if (ipAttach(0, devNamePrefix) == ERROR)
    {
        DRV_LOG_ARG("Failed to attach to device %s", devNamePrefix);
        return ERROR;
    }

    DRV_LOG("PciEndRetrieving IP()\n");

    if (OK == usrBootLineCrack(BOOT_LINE_ADRS, &params))
    {
        memcpy(myNetworkStr,params.bad,sizeof(myNetworkStr));

        (void)strtok(myNetworkStr, ":");

        myInet.s_addr = inet_addr(myNetworkStr);
    }
    

    DRV_LOG("PciEndifConfig()\n");
    if (usrNetIfConfig(devNamePrefix, 0, myNetworkStr, ifname, 0xfffffff0) == ERROR)
    {
        DRV_LOG_ARG("Failed to config I/f%s", devNamePrefix);
        return ERROR;
    }

    #ifdef M_TGT_L2
    peerInet.s_addr = myInet.s_addr - 1;
    #else
    peerInet.s_addr = myInet.s_addr + 1;
    #endif
    inet_ntoa_b(peerInet, PeerInetStr);

    networkInet = myInet;
    networkInet.s_addr &= 0xfffffff0;

    inet_ntoa_b(networkInet, networkStr );
    
    DRV_LOG("PciEndPeer Host\n");
    if(ERROR == hostAdd(PeerName, PeerInetStr))
    {
        DRV_LOG_ARG("Failed to add peer host%s", devNamePrefix);
        return ERROR;
    }

    return OK;
}

STATUS PciEndEnable(void)
{  
    return ifFlagChange("PciEnd",IFF_UP,TRUE);
}


STATUS PciEndDisable(void)
{  
    return ifFlagChange("PciEnd",IFF_UP,FALSE);
}

STATUS RegisterPciEndSend(PCISENDFUNC funcPtr)
{
    if ( NULL == PciEndSendFunc )
    {
        PciEndSendFunc = funcPtr;
        return OK;
    }
    DRV_LOG("PciEndSendFunc already registered\n");
    return ERROR;
}


STATUS  enablePciEndDebug()
{
    DebugPciEnd = TRUE;
    return OK;
}

STATUS disablePciEndDebug()
{
    DebugPciEnd = FALSE;
    return OK;
}
