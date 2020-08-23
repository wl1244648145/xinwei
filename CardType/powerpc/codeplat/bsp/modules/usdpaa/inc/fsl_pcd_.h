/******************************************************************************

 ?1995-2003, 2004, 2005-2010 Freescale Semiconductor, Inc.
 All rights reserved.

 This is proprietary source code of Freescale Semiconductor Inc.,
 and its use is subject to the NetComm Device Drivers EULA.
 The copyright notice above does not evidence any actual or intended
 publication of such source code.

 **************************************************************************/
#ifndef __FSL_PCD_API_H
#define __FSL_PCD_API_H



typedef enum e_EthId {
    e_ETH_ID_CORENET = 0, 
    e_ETH_ID_DEBUG,
    e_ETH_ID_ETH1,
    e_ETH_ID_ETH2,
    e_ETH_ID_NUM
} e_EthId;



int  BspModifyRouteKey(e_EthId  ucEthId, \
                       char     *acDestName, \
	                   unsigned char ucRelativeIndex, \
	                   unsigned char ucValidKeySize,  \
	                   char *pKey, \
	                   char *pMask);


int ChangeRoutToCcMaster(uint16_t wMasterCcBit);
int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr);
int ModifyRoutForDsp(uint8_t ucDspId, uint8_t* pMacAddr);
int ModifyRoutForUsDpaa(uint8_t* pMacAddr, uint32_t ucUsDpaaIp, uint16_t wUdpPortId);

#endif /* __FSL_PCD_API_H */

