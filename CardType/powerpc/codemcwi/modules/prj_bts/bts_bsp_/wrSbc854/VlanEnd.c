/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*******************************************************************
*
* FILENAME: VlanEnd.cpp
* DESCRIPTION:
*   This file contains the END driver for SAG & BTS communication.
*
*
* HISTORY:
*
*   Date       Author  Description
*   ---------  ---------  -------------------------------------------
*   10/16/2007   Xin Wang     Initial file creation.
*
*******************************************************************/
#if 0
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
#include "logLib.h"
#include "mcWill_bts.h"
#include "mv643xxEnd.h"
#endif
/*在此处声明，在OAMCFG  赋值*/

UINT32 g_SAG_VLAN_USAGE     = 0;
UINT32 g_SAG_VLAN_BTSIP     = 0;
UINT32 g_SAG_VLAN_ID        = 0;
UINT32 g_SAG_VLAN_GATEWAY   = 0;
UINT32 g_SAG_VLAN_SUBNETMASK= 0;
#if 0
typedef STATUS (*VLANSENDFUNC) (M_BLK_ID);/* pfunction returning int */

VLANSENDFUNC VlanEndSendFunc = NULL; /* implemented in vlanend.cpp */
extern int ipAttach(int unit, char* pDevice);
extern void DumpMbuf(M_BLK_ID pMblk);
extern STATUS mv643xxHandleVLANENDTx(M_BLK_ID pMblk);
extern STATUS sysSAGEnetAddrGet(UINT8 *adrs);
#if 0
extern void Drv_RegisterVLANEND(pfVLANENDRxCallBack );
extern void Drv_UnRegisterVLANEND(pfVLANENDRxCallBack );
#endif
#define ENET_SIZE             6       /* Ethernet address size */

#define END_HADDR(pEnd) ((pEnd)->pMib2Tbl->m2Data.mibIfTbl.ifPhysAddress.phyAddress)
#define END_HADDR_LEN(pEnd) ((pEnd)->pMib2Tbl->m2Data.mibIfTbl.ifPhysAddress.addrLength)

DRV_CTRL *  pDrvCtrl=NULL;

#undef VIRT_TO_PHYS
#define VIRT_TO_PHYS(vAddr)                                                \
(((int)pDrvCtrl->virtToPhys) ? (*pDrvCtrl->virtToPhys)(vAddr) : vAddr)

#undef PHYS_TO_VIRT
#define PHYS_TO_VIRT(pAddr)                                                \
(((int)pDrvCtrl->physToVirt) ? (*pDrvCtrl->physToVirt)(pAddr) : pAddr)

/* Global Variables*/
/*#define DEBUG_VLAN_END */
BOOL DebugVlanEnd = FALSE;
#ifdef DEBUG_VLAN_END
#define DRV_LOG(X0)     printf(X0)
#define DRV_LOG_ARG(X0, X1)    printf(X0,X1)
#else
#define DRV_LOG(X0)   ;
#define DRV_LOG_ARG(X0,X1) ;
#endif
char devNmPrefix[8] = "VlanEnd";
END_OBJ *VlanEndDrv = NULL;

/* This is the only externally visible interface. */
END_OBJ*         VlanEndLoad (char* initString);

void VlanShow(void); /* may be called from WindSh */

STATUS    VlanEndStart(END_OBJ* pEnd);
STATUS    VlanEndSend (END_OBJ *, M_BLK_ID);
int            VlanEndIoctl(END_OBJ* pEnd, int cmd, caddr_t data);
STATUS    VlanEndMCastAddrAdd (END_OBJ *, char *);
STATUS    VlanEndMCastAddrDel (END_OBJ *, char *);
STATUS    VlanEndMCastAddrGet (END_OBJ *, MULTI_TABLE *);
STATUS    VlanEndStop (END_OBJ *);
STATUS    VlanEndUnload(END_OBJ *);
/*STATUS    VlanEndUnload(DRV_CTRL *) ;*/
STATUS    VlanEndPollSend (END_OBJ *, M_BLK_ID);
STATUS    VlanEndPollRcv (END_OBJ *, M_BLK_ID);
STATUS    VlanEndmCastAddrAdd(END_OBJ* pEnd, char* addr);
STATUS    VlanEndmCastAddrDel(END_OBJ* pEnd, char* addr);
STATUS    VlanEndmCastAddrGet(END_OBJ* pEnd, MULTI_TABLE* tbl);

NET_FUNCS VlanEndFuncTbl = 
{
    VlanEndStart,
    VlanEndStop,
    VlanEndUnload,
    VlanEndIoctl,
    VlanEndSend,
    VlanEndmCastAddrAdd,
    VlanEndmCastAddrDel,
    VlanEndmCastAddrGet,
    VlanEndPollSend,
    VlanEndPollRcv,
    endEtherAddressForm,                /* put address info into a NET_BUFFER*/
    endEtherPacketDataGet,              /* get pointer to data in NET_BUFFER*/
    endEtherPacketAddrGet               /* Get packet addresses. */
};

#if 0
#define  VLANEND_DRV
#ifdef VLANEND_DRV
typedef void (*pfVLANENDRxCallBack)(char *, UINT16, char *);
pfVLANENDRxCallBack gSendToVLANEND=NULL;
void   Drv_RegisterVLANEND(pfVLANENDRxCallBack ENDFunc);
#endif
#endif

STATUS sysSAGEnetAddrGet
    (
    UINT8 *adrs
    )
{
	int byte;
	T_I2C_TABLE i2cData;
	i2c_read(I2C_E2PROM_DEV_ADDR, 0, &i2cData, sizeof(i2cData));
	bcopy(i2cData.L3_mac, adrs, 6);
	/* Check to see if mac address is valid. */
	for (byte=0; byte < 6; byte++)
	{
		if (adrs[byte] != 0xff)
			break;
	}

	if (byte == 6)
	{
		/* MAC address has ff:ff:ff:ff:ff:ff, copy default  address*/
		logMsg("Using default MAC address for the unit SAG enddriver\n",0,1,2,3,4,5);
	}	
#if DEBUG_VLAN_END
	else
	{
		logMsg( "MAC address for the unit SAG enddriver : \r\n%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", 
		i2cData.L3_mac[0],
		i2cData.L3_mac[1],
		i2cData.L3_mac[2],
		i2cData.L3_mac[3],
		i2cData.L3_mac[4],
		i2cData.L3_mac[5]);
	}
#endif	
	return(OK);
}

/*============================================================
MEMBER FUNCTION:
    CTBridge::RxDriverPacketCallBack

DESCRIPTION:
    接收网卡上送的报文

ARGUMENTS:
    char* : data to be sent.
    UINT16: data length
    char* : buffer ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
#if 0
/*add by wangx for test*/
static int j=0,count=0;
/*end*/
extern DRV_CTRL *pEBDrvCtrl;

void RxDriverPacketCallBack(char *pRxData, UINT16 usDataLength, char *pRxBuf)
{

     M_BLK_ID    pMblk   = (M_BLK_ID)NULL;       /* pointer to MBLK */
     END_OBJ *   pEndObj =&(pEBDrvCtrl->endObj);    /* pointer to END_OBJ */
     CL_BLK_ID   pClBlk;                         /* pointer to cluster block */
     /*logMsg("\r\nRxDriverPacketCallBack---<1>\r\n",1,2,3,4,5,6);*/

/*add by wangx for test*/  
#if 1              
UINT8 *p1 = pRxData;
if(*(p1+11) == 0xBA)
{     
	printf(" \r\nPacket [%d] Context : ",count);
	    for(j=0;j<20;j++,p1++)
	    	{
	    		printf("%.2x ",*p1);
	    	}
	count++;
	printf(" \r\n");
}

#endif  
/*add end*/	

     pMblk  = netMblkGet (pEndObj->pNetPool, M_DONTWAIT, MT_DATA);
     pClBlk = netClBlkGet (pEndObj->pNetPool,M_DONTWAIT);

     if ((pMblk == NULL) || (pRxData == NULL) || (pClBlk == NULL))
     {
		/**/logMsg("RxDriverPacketCallBack---<2>   pMblk = 0x%x   ---- pRxBuf = 0x%x   ---- pClBlk = 0x%x\n",pMblk,pRxBuf,pClBlk,4,5,6);     
		/* DRV_LOG (DRV_DEBUG_ERROR,
		                     "RxDriverPacketCallBack: NO BUFFERS %08x %08x %08x\n",
		                      (int)pMblk, (int)pRxData, (int)pClBlk, 4, 5, 6);*/
		END_ERR_ADD (pEndObj, MIB2_IN_ERRS, 1);
		if (pMblk)
			netMblkFree (pEndObj->pNetPool, pMblk);
		if (pClBlk)
			clBlkFree (pEndObj->pNetPool, pClBlk);
     }
     else
     {
		/* logMsg("\r\nRxDriverPacketCallBack---<3>---pRxBuf = %s\r\n",pRxBuf,2,3,4,5,6);*/
		/* join rx descriptor data buf to cBlk */
		netClBlkJoin (pClBlk, (char *)PHYS_TO_VIRT (pRxBuf),MV_BUFSIZ, NULL, 0, 0, 0);
		/* associate the CLBlk with the MBLK */
		netMblkClJoin (pMblk, pClBlk);
		pMblk->mBlkHdr.mLen    = usDataLength;
		pMblk->mBlkPktHdr.len  = usDataLength;
		pMblk->mBlkHdr.mFlags |= M_PKTHDR;

		/* 643xx puts rx data starting at an offset of 2 into the
		 * the receive buffer.  adding 2 to the buf_ptr below
		 * compensates for this.
		 */
		pMblk->mBlkHdr.mData  = (char *)(PHYS_TO_VIRT (pRxBuf+ ETHER_RXBUF_PTR_ADJUST));

		/**************************************************************/
		/*****将这里转向到VlanEnd去*********VlanEndDrv**********/
#if 1
		END_ERR_ADD (pEndObj, MIB2_IN_UCAST, 1);
		END_RCV_RTN_CALL (pEndObj, pMblk);
#else
		END_ERR_ADD (VlanEndDrv, MIB2_IN_UCAST, 1);
		END_RCV_RTN_CALL (VlanEndDrv, pMblk);
#endif
		/*DRV_LOG (DRV_DEBUG_RX, "RxDriverPacketCallBack: RDR=%x len %x\n",(int)pRxBuf, usDataLength, 3, 4, 5, 6);*/
		/*logMsg("recv Data From drv",1,2,3,4,5,6);*/    	
	}	   
}
#endif

 
/*******************************************************************************
*
* VlanEndLoad - initialize the driver and device
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
END_OBJ* VlanEndLoad
(
    char* initString
   
)
{

	END_OBJ *pEnd;
	STATUS rtv;
	UINT8 addr[6] = {0x99,0x88,0x77,0x66,0x55,0x44};
#ifdef DEBUG_VLAN_END
	logMsg("\r\nVlanEndLoad\n",1,2,3,4,5,6);
#endif
	DRV_LOG("\r\nVlanEndLoad\n");

	if(initString == NULL)
	{
		DRV_LOG("\r\nInvalid argument for Load\r\n");
		return(NULL);
	}

	if(initString[0] == 0)
	{
		DRV_LOG("\r\nGetting Driver name...\r\n");
		memcpy(initString, devNmPrefix, strlen(devNmPrefix));
		return NULL;
	}

	DRV_LOG ("\r\nLoading SAG End Driver...\r\n");

	/* Parse the initialization string */
	pEnd = (END_OBJ*)calloc(sizeof(END_OBJ),1);
	if(NULL == pEnd)
	{
		DRV_LOG("\r\nVlanEndLoad Cannot allocate END Device control block\r\n");
		return(NULL);
	}

	pEnd->pFuncTable = &VlanEndFuncTbl;
	strcpy(pEnd->devObject.name, devNmPrefix);
	pEnd->devObject.unit = 0;
	strcpy(pEnd->devObject.description, "SAG VLAN END Driver.");

	pEnd->attached = FALSE;
	pEnd->flags    = 0;
	pEnd->txSem    = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);

	/* Initialize END object data structures */
	if(END_OBJ_INIT(pEnd, (DEV_OBJ*)&pEnd->devObject, devNmPrefix, 0, &VlanEndFuncTbl, "SAG END Driver") == ERROR)
	{
		DRV_LOG("\r\nSAG END Driver initi failed\r\n");
		free ((char *)pEnd);
		return NULL;
	}

	/*这里要保证SAG VLAN Enddriver 协议栈MAC Address 与MV0的协议栈MAC Address*/
	/*相同，若出现不同则可能是I2C驱动问题，关电重启可以解决问题*/
	sysSAGEnetAddrGet(&addr[0]);

	rtv = END_MIB_INIT (pEnd,
			                  M2_ifType_ethernet_csmacd,
			                  addr,
			                  ENET_SIZE,
			                  ETHERMTU,
			                  20000000);  /* 20Mbps */
	END_OBJ_READY (pEnd, IFF_NOTRAILERS | IFF_MULTICAST | IFF_BROADCAST);

	if (endFindByName ( initString, 0) != NULL)
	{
		DRV_LOG_ARG("can not find SAG end %s\n", initString);
	}

	if (endFindByName ( initString, 1) != NULL)
	{
		DRV_LOG_ARG("can not find SAG end %s\n", initString);
	}
	DRV_LOG("endof SAG endload\n");

	/*  Drv_RegisterVLANEND(RxDriverPacketCallBack); */
	VlanEndDrv = pEnd;
	return pEnd;

}


STATUS startvlan()
{
    StartVlanEnd();
    g_SAG_VLAN_USAGE = 1;   
    return OK;
}
STATUS stopvlan()
{
    /*StopVlanEnd();*/
    g_SAG_VLAN_USAGE = 0;  	
    return OK;
}

STATUS showvlancfg()
{
	printf("\r\n************sag vlan config ***************************\r\n");
	printf("\r\n  g_SAG_VLAN_USAGE	: 0x%x\r\n",g_SAG_VLAN_USAGE);
	printf("\r\n  g_SAG_VLAN_BTSIP	: 0x%x\r\n",g_SAG_VLAN_BTSIP);
	printf("\r\n  g_SAG_VLAN_ID		: 0x%x\r\n",g_SAG_VLAN_ID);	
	printf("\r\n  g_SAG_VLAN_GATEWAY	: 0x%x\r\n",g_SAG_VLAN_GATEWAY);
	printf("\r\n  g_SAG_VLAN_SUBNETMASK	: 0x%x\r\n",g_SAG_VLAN_SUBNETMASK);		
	printf("\r\n****************************************************\r\n");
	return OK;
}

STATUS VlanEndStart(END_OBJ* pEnd)
{
#ifdef DEBUG_VLAN_END
	logMsg("VlanEndStart\n",1,2,3,4,5,6);
#endif
	DRV_LOG("VlanEndStart\n");

	if(NULL == pEnd)
	{
		DRV_LOG("Bad END Obj\n");
		return ERROR;
	}

	END_FLAGS_SET(pEnd, (IFF_UP | IFF_RUNNING));
	return OK;
}

STATUS VlanEndStop(END_OBJ* pEnd)
{
#ifdef DEBUG_VLAN_END
	logMsg("VlanEndStop\n",1,2,3,4,5,6);
#endif
	DRV_LOG("VlanEndStop\n");

	if(NULL == pEnd)
	{
		DRV_LOG("Bad END Obj\n");
		return ERROR;
	}

	return OK;
}

STATUS VlanEndUnload(END_OBJ* pEnd)
{
	DRV_LOG("VlanEndUnload\n");
#ifdef DEBUG_VLAN_END	
	logMsg("VlanEndUnload()\n",1,2,3,4,5,6);
#endif
	/* free lists */
	END_OBJECT_UNLOAD (pEnd);
	if(NULL == pEnd)
	{
		DRV_LOG("Bad END Obj\n");
		return ERROR;
	}
	return OK;
}


int VlanEndIoctl(END_OBJ* pEnd, int cmd, caddr_t data)
{
	int value;
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndIoctl - %08x \n",cmd,2,3,4,5,6);
#endif
	DRV_LOG_ARG("VlanEndIoctl - %08x ", cmd);

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


STATUS VlanEndSend(END_OBJ* pEnd, M_BLK_ID pMblk)
{
	DRV_LOG("VlanEndSend\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndSend\n",1,2,3,4,5,6);
#endif
	if(NULL == pMblk)
	{
		DRV_LOG("Bad MBLK\n");
		return ERROR;
	}

	DumpMbuf(pMblk);
	mv643xxHandleVLANENDTx(pMblk);
	return OK;
	
	if (mv643xxHandleVLANENDTx)
	{
		return mv643xxHandleVLANENDTx(pMblk);
	}
	else
	{
		DRV_LOG("Send IP packet through SAG END before ready\n");
		return ERROR;
	}
}


STATUS VlanEndmCastAddrAdd(END_OBJ* pEnd, char* addr)
{
	DRV_LOG("VlanEndmCastAddrAdd\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndmCastAddrAdd\r\n",1,2,3,4,5,6);
#endif
	return ERROR;
}

STATUS VlanEndmCastAddrDel(END_OBJ* pEnd, char* addr)
{
	DRV_LOG("VlanEndmCastAddrDel\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndmCastAddrDel\r\n",1,2,3,4,5,6);
#endif
	return ERROR;
}

STATUS VlanEndmCastAddrGet(END_OBJ* pEnd, MULTI_TABLE* tbl)
{
	DRV_LOG("VlanEndmCastAddrGet\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndmCastAddrGet\r\n",1,2,3,4,5,6);
#endif
	return ERROR;
}

STATUS VlanEndPollSend(END_OBJ* pEnd, M_BLK_ID pMblk)
{
	DRV_LOG("VlanEndPollSend\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndPollSend\r\n",1,2,3,4,5,6);
#endif	
	return ERROR;
}

STATUS VlanEndPollRcv(END_OBJ* pEnd, M_BLK_ID pMblk)
{
	DRV_LOG("VlanEndPollRcv\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndPollRcv\r\n",1,2,3,4,5,6);
#endif
	return ERROR;
}

M_BLK_ID VlanEndFormAddress(M_BLK_ID pMblk, M_BLK_ID pMblkSrc, M_BLK_ID pMblkDst, int tbd)
{
	DRV_LOG("VlanEndFormAddress\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndFormAddress\n",1,2,3,4,5,6);
#endif
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

STATUS VlanEndPacketDataGet(M_BLK_ID pMblk, LL_HDR_INFO* llHdr)
{
	DRV_LOG("VlanEndPacketDataGet\n");
#ifdef DEBUG_VLAN_END		
	logMsg("VlanEndPacketDataGet\r\n",1,2,3,4,5,6);
#endif
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
	llHdr->destSize           = 6;
	llHdr->srcAddrOffset    = 6;
	llHdr->srcSize             = 6;
	llHdr->ctrlAddrOffset    = 1;
	llHdr->ctrlSize             = pMblk->mBlkPktHdr.len; 
	llHdr->pktType            = (pMblk->mBlkHdr.mData[12] << 8) | pMblk->mBlkHdr.mData[13];
	llHdr->dataOffset         = SIZEOF_ETHERHEADER;
	return OK;
}

STATUS VlanEndEnable(void)
{  
#ifdef DEBUG_VLAN_END	
	logMsg("VlanEndEnable\r\n",1,2,3,4,5,6);
#endif
	return ifFlagChange("VlanEnd",IFF_UP,TRUE);
}

STATUS VlanEndDisable(void)
{  
#ifdef DEBUG_VLAN_END	
	logMsg("VlanEndDisable\r\n",1,2,3,4,5,6);
#endif
	return ifFlagChange("VlanEnd",IFF_UP,FALSE);
}

STATUS  enableVlanEndDebug()
{
#ifdef DEBUG_VLAN_END	
	logMsg("enableVlanEndDebug\r\n",1,2,3,4,5,6);
#endif
	DebugVlanEnd = TRUE;
	return OK;
}

STATUS disableVlanEndDebug()
{
#ifdef DEBUG_VLAN_END	
	logMsg("disableVlanEndDebug\r\n",1,2,3,4,5,6);
#endif
	DebugVlanEnd = FALSE;
	return OK;
}

char  myNetworkStr  [BOOT_TARGET_ADDR_LEN];
char  vlanEndPeerName[8] = "Gateway";
char  PeerInetStr[INET_ADDR_LEN];
char  vlanEndNamePrefix[8] = "VlanEnd";

STATUS StartVlanEnd(/*NET_POOL_ID pNetPool*/)
{      
	END_OBJ *myEnd;
	char ifname[20];

	struct in_addr ipaddr;
	UINT32 subnetMask;  
	char myIpAddress[ INET_ADDR_LEN ] = {0};
	char myGatewayIpAddr[ INET_ADDR_LEN ] = {0};

	ipaddr.s_addr = htonl(g_SAG_VLAN_BTSIP);
	inet_ntoa_b( ipaddr, myIpAddress );
	ipaddr.s_addr = htonl(g_SAG_VLAN_GATEWAY);
	inet_ntoa_b( ipaddr, myGatewayIpAddr );	
	subnetMask = g_SAG_VLAN_SUBNETMASK;	
#ifdef DEBUG_VLAN_END	
	logMsg("StartVlanEnd\r\n",1,2,3,4,5,6);
#endif
	strcpy(ifname, vlanEndNamePrefix);

	myEnd = endFindByName(vlanEndNamePrefix,0);
	if (ipAttach(0, vlanEndNamePrefix) == ERROR)
	{
		DRV_LOG_ARG("Failed to attach to device %s", vlanEndNamePrefix);
		return ERROR;
	}
	/*routeShow();*/
	DRV_LOG("VlanEndRetrieving IP()\n");
#ifdef DEBUG_VLAN_END		
	logMsg("SAG Vlan Endriver's IP address: %s\n",(int)myIpAddress,2,3,4,5,6);
	logMsg("SAG Vlan Endriver's Gateway address: %s\n",(int)myGatewayIpAddr,2,3,4,5,6);
	logMsg("SAG Vlan Endriver's Subnet Mask : 0x%x\n",(int)g_SAG_VLAN_SUBNETMASK,2,3,4,5,6);
#endif	
	
	if (usrNetIfConfig(vlanEndNamePrefix, 0, myIpAddress, ifname, subnetMask) == ERROR)
	{
		DRV_LOG_ARG("Failed to config I/f%s\n", vlanEndNamePrefix);
#ifdef DEBUG_VLAN_END			
		logMsg("Failed to config I/f%s\n",(int)vlanEndNamePrefix,2,3,4,5,6);
#endif
		return ERROR;
	}

/*this is for test ,add a default gateway*/	
#if 0
	if (usrNetIfConfig(vlanEndNamePrefix, 0, myGatewayIpAddr, ifname, 0) == ERROR)
	{
		DRV_LOG_ARG("Failed to config I/f%s", vlanEndNamePrefix);
		logMsg("Failed to config I/f%s",vlanEndNamePrefix,2,3,4,5,6);
		return ERROR;
	}
#endif	
/*end for test*/

#if 0
	myInet.s_addr = inet_addr(myNetworkStr);
#ifdef M_TGT_L2
	peerInet.s_addr = myInet.s_addr - 1;
#else
	peerInet.s_addr = myInet.s_addr + 1;
#endif
	inet_ntoa_b(peerInet, PeerInetStr);

	networkInet = myInet;
	networkInet.s_addr &= 0xffff0000;

	inet_ntoa_b(networkInet, networkStr );

	/*hostShow();   */
	DRV_LOG("VlanEndPeer Host\n");
#endif

	if(ERROR == hostAdd(vlanEndPeerName, myGatewayIpAddr))
	{
		DRV_LOG_ARG("Failed to add peer host%s\n", vlanEndNamePrefix);
		return ERROR;
	}
	
#ifdef DEBUG_VLAN_END	
	routeShow();    	
	hostShow();
#endif

	return OK;
}

#if 0
STATUS StopVlanEnd(/*NET_POOL_ID pNetPool*/)
{      
	END_OBJ *myEnd;
	myEnd = endFindByName(vlanEndNamePrefix,0);
	if (ipDetach(0, vlanEndNamePrefix) == ERROR)
	{
		DRV_LOG_ARG("Failed to attach to device %s\n", vlanEndNamePrefix);
		return ERROR;
	}

	/**/routeShow();    	
	/**/hostShow();
	return OK;
}
#endif
#endif