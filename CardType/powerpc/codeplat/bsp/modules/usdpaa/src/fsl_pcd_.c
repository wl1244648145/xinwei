#include <string.h>

#include "../inc/fm_lib.h"
#include "../inc/fsl_pcd_api.h"
#include "../inc/fm_pcd_ioctls.h"

//unsigned char aucUnicastMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
const unsigned char gaucUnicastMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
{
    {/* Mask */
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Dst MAC Mask */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Src MAC Mask */\
        0xFF,0xFF,/* 16bit MAC type Mask */\
        0x00,0x00,0x00,0x00,/* 4bit版本号+4bit首部长度+8bitTOS+16bit总长度 Mask */\
        0x00,0x00,0x00,0x00,/* 16bit标识+3bit标志+13bit片偏移 Mask */\
        0x00,0x00,0x00,0x00,/* 8bit TTL+8bit协议+16bit首部校验和 Mask */\
        0x00,0x00,0x00,0x00,/* 32bit Src IP Mask */\
        0xFF,0xFF,0xFF,0xFF,/* 32bit Dst IP Mask */\
        0x00,0x00,0x00,0x00,/*  */\
        0x00,0x00,0x00,0x00 /*  */
    },
    {/* Key */
        0x40,0x00,0xc0,0x00,0x06,0x11,/* 48bit Dst MAC */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Src MAC */\
        0x08,0x00,/* 16bit MAC type */\
        0x00,0x00,0x00,0x00,/* 4bit版本号+4bit首部长度+8bitTOS+16bit总长度 */\
        0x00,0x00,0x00,0x00,/* 16bit标识+3bit标志+13bit片偏移 */\
        0x00,0x00,0x00,0x00,/* 8bit TTL+8bit协议+16bit首部校验和 */\
        0x00,0x00,0x00,0x00,/* 32bit Src IP */\
        0xc0,0xfe,0x06,0x10,/* 32bit Dst IP */\
        0x00,0x00,0x00,0x00,/*  */\
        0x00,0x00,0x00,0x00 /*  */
    },
};

/* no vlan arp request multicast */
//unsigned char aucMultiArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
const unsigned char gaucMultiArpMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
{
    {/* Mask */
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,/* 48bit Dst MAC */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Src MAC */\
        0xFF,0xFF,/* 16bit MAC type */\
        0x00,0x00,0x00,0x00,/* Hardware type + Protocol type */\
        0xFF,0xFF,0xFF,0xFF,/* Hard address + Portocol address len + Operation Code */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Sender's hard address */\
        0x00,0x00,0x00,0x00,          /* Sender's protocol address */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Target's hard address */\
        0xFF,0xFF,0xFF,0xFF           /* Target's Protocal address */
    },
    {/* Key */
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,/* 48bit Dst MAC */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Src MAC */\
        0x08,0x06,/* 16bit MAC type */\
        0x00,0x00,0x00,0x00,/* Hardware type + Protocol type */\
        0x06,0x04,0x00,0x01,/* Hard address + Portocol address len + Operation Code */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Sender's hard address */\
        0x00,0x00,0x00,0x00,          /* Sender's protocol address */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Target's hard address */\
        0xc0,0xfe,0x06,0x10           /* Target's Protocal address */
    },
};

/* no vlan unicast arp reply and unicast arp request */
//unsigned char aucUniArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
const unsigned char gaucUniArpMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
{
    {/* Mask */
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Dst MAC */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Src MAC */\
        0xFF,0xFF,/* 16bit MAC type */\
        0x00,0x00,0x00,0x00,/* Hardware type + Protocol type */\
        0xFF,0xFF,0xFF,0x00,/* Hard address + Portocol address len + Operation Code */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Sender's hard address */\
        0x00,0x00,0x00,0x00,          /* Sender's protocol address */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Target's hard address */\
        0xFF,0xFF,0xFF,0xFF           /* Target's Protocal address */
    },
    {/* Key */
        0x40,0x00,0xc0,0x00,0x06,0x10,/* 48bit Dst MAC */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* 48bit Src MAC */\
        0x08,0x06,/* 16bit MAC type */\
        0x00,0x00,0x00,0x00,/* Hardware type + Protocol type */\
        0x06,0x04,0x00,0x02,/* Hard address len + Portocol address len + Operation Code */\
        0x00,0x00,0x00,0x00,0x00,0x00,/* Sender's hard address */\
        0x00,0x00,0x00,0x00,          /* Sender's protocol address */\
        0x40,0x00,0xc0,0x00,0x06,0x10,/* Target's hard address */\
        0xc0,0xfe,0x06,0x10           /* Target's Protocal address */
    },
};


/* no vlan unicast arp reply and unicast arp request */
const unsigned char aucMatchNonMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
{
    MASK_KEY_MATCH_NON
};

/* no vlan unicast arp reply and unicast arp request */
const unsigned char aucMatchAllMaskAndKey[2][FM_PCD_MAX_SIZE_OF_KEY] = 
{
    MASK_KEY_MACH_ALL
};


int CheckDestName(char *acDestName)
{
    unsigned int dwLoop;
    
    for(dwLoop = 0; dwLoop < sizeof(((ioc_fm_CcNodeHandle_params_t*)0)->acDestName); dwLoop++)
    {
        if(0 == acDestName[dwLoop])
        {
            break;
        }
    }
    if(dwLoop == sizeof(((ioc_fm_CcNodeHandle_params_t*)0)->acDestName))
    {
        printf("[debug] BspModifyRouteKey input acDestName:%s too long!\n", acDestName);
        return ~0x0;
    }
    else
    {
        return 0;
    }
}


/* 可用目的name:
"To USDPAA", "To Linux", "To LWOS1", "To LWOS2", "To LWOS3", "To LWOS4", "To LWOS5", 
"To LWOS6", "To LWOS7", "To DSPA", "To DSPB", "To FPGA", "To Drop", "To CC_0",  "To CC_1" */

int  BspModifyRouteKey(e_EthId  ucEthId, \
                       char     *acDestName, \
                       unsigned char ucRelativeIndex, \
                       unsigned char ucValidKeySize,  \
                       char *pKey, \
                       char *pMask)
{
    t_Error ret = 0;
    uint8_t ucFManDevId;
    uint8_t ucPortDevId;
    t_Handle h_engines;
    t_Handle h_CcNodeHandle;
    uint32_t dwCheckNameResult;
    uint8_t  ucRouteIndexBase;

    ucFManDevId = EthId2FManDevId(ucEthId);
	printf("hahahahhahahahah ucFManDevId->0x%lx\n",ucFManDevId);
    if(0xff == ucFManDevId)
    {
        return ~0x0;
    }
    ucPortDevId = EthId2PortDevId(ucEthId);
    if(0xff == ucPortDevId)
    {
        return ~0x0;
    }
    dwCheckNameResult = CheckDestName(acDestName);
    if(0 != dwCheckNameResult)
    {
        return dwCheckNameResult;
    }
	//ucEthId:0,ucFManDevId:1,ucPortDevId:0     
    printf("ucEthId:%d,ucFManDevId:%d,ucPortDevId:%d\n", (uint32_t)ucEthId, ucFManDevId, ucPortDevId);
    /* Open/Assign FM device */
    h_engines = (t_Handle)FM_Open(ucFManDevId);
    if ( 0 == h_engines ) {
        printf("[debug] 0 == engines[0] error!\n");
        return ~0x0;
    }
    else
    {
        printf("[debug] 0 == engines[0] OK!\n");
    }

    h_CcNodeHandle = (t_Handle)FM_PCD_GetCcNodeInfo(h_engines,ucFManDevId,ucPortDevId, acDestName, 0, &ucRouteIndexBase);
    if(0 == h_CcNodeHandle)
    {
        printf ("get CcNode info failed\n");
    }
    else
    {
        printf ("get CcNode info OK\n");
    }

    ret = FM_PCD_CcNodeModifyKey(h_engines, h_CcNodeHandle, ucRouteIndexBase + ucRelativeIndex, 42, pKey, pMask);
    if (ret != 0)
    {
        printf ("Failed to modify key at runtime!, error = %d\r\n", ret);
    }
    else
    {
        printf ("Port Number changed\r\n");
    }

    /* Close handles */
    FM_Close( h_engines );
    return 0;
}

//0x48寄存器，bit1为1时表示1号槽位的cc是主用，bit2为1时表示2号槽位的cc是主用cc
int ChangeRoutToCcMaster(uint16_t wMasterCcBit)
{
    uint32_t ucLoop;
    
    if(0x2 == (wMasterCcBit&0x6))
    {
        for(ucLoop = e_ETH_ID_ETH1; ucLoop < e_ETH_ID_NUM; ucLoop++)
        {
            BspModifyRouteKey(ucLoop, \
                       "To CC_0", \
                       0, \
                       42,  \
                       aucMatchAllMaskAndKey[1], \
                       aucMatchAllMaskAndKey[0]);
        }
    }
    else if(0x4 == (wMasterCcBit&0x6))
    {
        for(ucLoop = e_ETH_ID_ETH1; ucLoop < e_ETH_ID_NUM; ucLoop++)
        {
            BspModifyRouteKey(ucLoop, \
                       "To CC_0", \
                       0, \
                       42,  \
                       aucMatchNonMaskAndKey[1], \
                       aucMatchNonMaskAndKey[0]);
        }
    }
    else
    {
        printf("[debug] error MasterCcBit:0x%x\n",wMasterCcBit);
    }
}


int ModifyRoutForSlaveCore(uint8_t ucSlaveCoreId, uint8_t* pMacAddr)
{
    uint32_t ucLoop;
    unsigned char *pMacH4Byte;
    unsigned char *pMacL2Byte;
    uint32_t dwMacH4Byte;
    uint32_t dwMacL2Byte;
    uint32_t dwRet;
    unsigned char ucMacAddr[6] = {0};
    unsigned char ucIpAddr[4]  = {0};
    char auDestName[] = "To LWOSx";
    unsigned char aucUnicastMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
    unsigned char aucMultiArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];	
    unsigned char aucUniArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
	
    BspGetDataNetMac(ucMacAddr);
    
    if((0 == ucSlaveCoreId)||(7 < ucSlaveCoreId))
    {
        return ~0x0;
    }

    auDestName[7] = ('0' + ucSlaveCoreId);
    
    memcpy(ucMacAddr, pMacAddr, 6);
    
    memcpy(ucIpAddr, &ucMacAddr[2],sizeof(ucIpAddr));

    memcpy(&aucUnicastMaskAndKeyTemp[0][0],  &gaucUnicastMaskAndKey[0][0],  sizeof(gaucUnicastMaskAndKey));
    memcpy(&aucMultiArpMaskAndKeyTemp[0][0], &gaucMultiArpMaskAndKey[0][0], sizeof(gaucMultiArpMaskAndKey));
    memcpy(&aucUniArpMaskAndKeyTemp[0][0],   &gaucUniArpMaskAndKey[0][0],   sizeof(gaucUniArpMaskAndKey));

    memcpy(&aucUnicastMaskAndKeyTemp[1][0],   ucMacAddr, 6);
    memcpy(&aucUnicastMaskAndKeyTemp[1][30],  ucIpAddr, 4);
    
    memcpy(&aucMultiArpMaskAndKeyTemp[1][38], ucIpAddr, 4);
    
    memcpy(&aucUniArpMaskAndKeyTemp[1][0],    ucMacAddr, 6);
    memcpy(&aucUniArpMaskAndKeyTemp[1][38],   ucIpAddr, 4);

    for(ucLoop = e_ETH_ID_CORENET; ucLoop < e_ETH_ID_NUM; ucLoop++)
    {
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   0, \
                   42,  \
                   aucUnicastMaskAndKeyTemp[1], \
                   aucUnicastMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for Unicast, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   1, \
                   42,  \
                   aucMultiArpMaskAndKeyTemp[1], \
                   aucMultiArpMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for MultiArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);

        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   2, \
                   42,  \
                   aucUniArpMaskAndKeyTemp[1], \
                   aucUniArpMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for UniArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);

    }

    return 0;
}

int ModifyRoutForDsp(uint8_t ucDspId, uint8_t* pMacAddr)
{
    uint32_t ucLoop;
    unsigned char *pMacH4Byte;
    unsigned char *pMacL2Byte;
    uint32_t dwMacH4Byte;
    uint32_t dwMacL2Byte;
    uint32_t dwRet;
    unsigned char ucMacAddr[6] = {0};
    unsigned char ucIpAddr[4]  = {0};
    char auDestName[] = "To DSPx";
    unsigned char aucUnicastMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
    unsigned char aucMultiArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];	
    unsigned char aucUniArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
	
    if(1 < ucDspId)
    {
        return ~0x0;
    }

    auDestName[6] = ('A' + ucDspId);
    
    memcpy(ucMacAddr, pMacAddr, 6);
    
    memcpy(ucIpAddr, &ucMacAddr[2],sizeof(ucIpAddr));

    memcpy(&aucUnicastMaskAndKeyTemp[0][0],  &gaucUnicastMaskAndKey[0][0],  sizeof(gaucUnicastMaskAndKey));
    memcpy(&aucMultiArpMaskAndKeyTemp[0][0], &gaucMultiArpMaskAndKey[0][0], sizeof(gaucMultiArpMaskAndKey));
    memcpy(&aucUniArpMaskAndKeyTemp[0][0],   &gaucUniArpMaskAndKey[0][0],   sizeof(gaucUniArpMaskAndKey));
    
    memcpy(&aucUnicastMaskAndKeyTemp[1][0],   ucMacAddr, 6);
    memcpy(&aucUnicastMaskAndKeyTemp[1][30],  ucIpAddr, 4);
    //aucUnicastMaskAndKeyTemp[0][5]  = 0b11100000;
    aucUnicastMaskAndKeyTemp[0][33] = 0xF0;

    memcpy(&aucMultiArpMaskAndKeyTemp[1][38], ucIpAddr, 4);
    aucMultiArpMaskAndKeyTemp[0][41] = 0xF0;
    
    memcpy(&aucUniArpMaskAndKeyTemp[1][0],    ucMacAddr, 6);
    memcpy(&aucUniArpMaskAndKeyTemp[1][38],   ucIpAddr, 4);
    //aucUniArpMaskAndKeyTemp[0][5]  = 0b11100000;
    aucUniArpMaskAndKeyTemp[0][41] = 0xF0;

    for(ucLoop = e_ETH_ID_CORENET; ucLoop < e_ETH_ID_NUM; ucLoop++)
    {
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   0, \
                   42,  \
                   aucUnicastMaskAndKeyTemp[1], \
                   aucUnicastMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for Unicast, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   1, \
                   42,  \
                   aucMultiArpMaskAndKeyTemp[1], \
                   aucMultiArpMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for MultiArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   2, \
                   42,  \
                   aucUniArpMaskAndKeyTemp[1], \
                   aucUniArpMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for UniArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        
    }

    return 0;
}

int ModifyRoutForUsDpaa(uint8_t* pMacAddr, uint32_t ucUsDpaaIp, uint16_t wUdpPortId)
{
    uint32_t ucLoop;
    uint32_t dwRet;
    unsigned char ucMacAddr[6] = {0};
    unsigned char ucIpAddr[4]  = {0};
    char auDestName[] = "To USDPAA";
    unsigned char aucUnicastMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
    unsigned char aucMultiArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];	
    unsigned char aucUniArpMaskAndKeyTemp[2][FM_PCD_MAX_SIZE_OF_KEY];
	
    if(0 == ucUsDpaaIp)
    {
        return ~0x0;
    }
    
    memcpy(ucMacAddr, pMacAddr, 6);
    memcpy(ucIpAddr, &ucUsDpaaIp, 4);

    memcpy(&aucUnicastMaskAndKeyTemp[0][0],  &gaucUnicastMaskAndKey[0][0],  sizeof(gaucUnicastMaskAndKey));
    memcpy(&aucMultiArpMaskAndKeyTemp[0][0], &gaucMultiArpMaskAndKey[0][0], sizeof(gaucMultiArpMaskAndKey));
    memcpy(&aucUniArpMaskAndKeyTemp[0][0],   &gaucUniArpMaskAndKey[0][0],   sizeof(gaucUniArpMaskAndKey));
    
    memcpy(&aucUnicastMaskAndKeyTemp[1][0],   ucMacAddr, 6);
    memcpy(&aucUnicastMaskAndKeyTemp[1][30],  ucIpAddr, 4);

    memcpy(&aucMultiArpMaskAndKeyTemp[1][38], ucIpAddr, 4);
    
    memcpy(&aucUniArpMaskAndKeyTemp[1][0],    ucMacAddr, 6);
    memcpy(&aucUniArpMaskAndKeyTemp[1][38],   ucIpAddr, 4);

    for(ucLoop = e_ETH_ID_CORENET; ucLoop < e_ETH_ID_NUM; ucLoop++)
    {
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   0, \
                   42,  \
                   aucUnicastMaskAndKeyTemp[1], \
                   aucUnicastMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for Unicast, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   1, \
                   42,  \
                   aucMultiArpMaskAndKeyTemp[1], \
                   aucMultiArpMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for MultiArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        dwRet = BspModifyRouteKey(ucLoop, \
                   auDestName, \
                   2, \
                   42,  \
                   aucUniArpMaskAndKeyTemp[1], \
                   aucUniArpMaskAndKeyTemp[0]);
        if(0 != dwRet)
            printf("[debug] BspModifyRouteKey() error for UniArp, EthId:0x%x, DestName:%s, ret:0x%x\n",ucLoop, auDestName, dwRet);

        bsp_sys_msdelay(5);
        
    }

    return 0;
}
#if 0
main()
{
    int dwResult = 0;
    
    printf("Begine modify CC route OK\n");

    while(1)
    {
        dwResult = BspModifyRouteKey(e_ETH_ID_CC0, \
                                     "To Drop", \
                                     0, \
                                     42,  \
                                     aucMatchNonMaskAndKey[1], \
                                     aucMatchNonMaskAndKey[0]);

        if(0 != dwResult)
        {
            printf("BspModifyRouteKey to match Non error dwResult:%u\n",dwResult);
        }
        else
        {
            printf("BspModifyRouteKey to match Non OK\n");
        }

        dwResult = BspModifyRouteKey(e_ETH_ID_CC0, \
                                     "To Drop", \
                                     0, \
                                     42,  \
                                     aucMatchAllMaskAndKey[1], \
                                     aucMatchAllMaskAndKey[0]);

        if(0 != dwResult)
        {
            printf("BspModifyRouteKey to match all error dwResult:%u\n",dwResult);
        }
        else
        {
            printf("BspModifyRouteKey OK\n");
        }
    }
    return ;
}

#endif

