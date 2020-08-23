/* usrAppInit.c - stub application initialization routine */

/* Copyright 1984-1998 Wind River Systems, Inc. */

/*
modification history
--------------------
01a,02jun98,ms   written
*/

/*
DESCRIPTION
Initialize user application code.
*/ 

/******************************************************************************
*
* usrAppInit - initialize the users application
*/ 
#include <vxworks.h>
#include <shellLib.h>
#include <stdio.h>
#include <vmLib.h>
#include <mcWill_bts.h>
#include <bootLib.h>
#include <time.h>

#define APP_LOADER_TASK_PRIORITY 65

LOCAL  unsigned char defaultBtsAdrs[2][6] = 
{
    { MCWILL_ENET0, MCWILL_ENET1, MCWILL_ENET2, 0xfa, 0, 5},
    { MCWILL_ENET0, MCWILL_ENET1, MCWILL_ENET2, 0xfa, 0, 9},
};
typedef enum 
{
    L3BOOTLINEFILE = 0,
    L2BOOTLINEFILE,
    NVEMSDATA,
    BTSPARA
}DATAFROMCF;

extern BOOL	  ataDrvInstalled;
char BtsMacGet (  int unit, UINT8 *adrs);
void getDataFromCF(char *filename, char *nvaddr, int type, char first);
extern int main();
extern int reset_BTS();
extern void sysNvDataModifyHandle(); 
extern void check_i2c_eeprom();
extern void StartWdtTask();
extern STATUS bspCreateRamDisk();
extern STATUS routeDelete
    (
    char * destination,       /* inet addr or name of route destination */
    char * gateway            /* inet addr or name of gateway to destination */
    );
extern void inet_netof_string
    (
    char *inetString,   /* inet addr to extract local portion from */
    char *netString     /* net inet address to return */
    );
extern STATUS ifMaskGet
    (
    char * interfaceName,     /* name of interface, i.e. ei0 */
    int *  netMask            /* buffer for subnet mask */
    );
#include <in.h>

LOCAL char cfMacFilename[20] = "/ata0a/btsMac";
LOCAL char l3FileName[20] = "/ata0a/l3Bootline";
LOCAL char l2FileName[20] = "/ata0a/l2Bootline";
LOCAL char FileName[20] = "/ata0a/nvEmsData";    
LOCAL char btsFileName[20] = "/ata0a/btsPara";
LOCAL char ctemp[6];
LOCAL unsigned char btsMac[6];
#ifndef WBBU_CODE
/*��Դ���ƣ���909��Ŀ����ֲ����*/
#define MV64360_GPP_VAL                 (0xf104)      
#define MV64360_BASE_ADRS  (0x14000000)
#undef  EIEIO
#define EIEIO              WRS_ASM (" eieio")

/* Read, write macros for system MV64360 controller */

#define MV64360_REG_RD(x,val)	\
	{ \
		UINT32 tmpVar ; \
		tmpVar = *((volatile UINT32*)(MV64360_BASE_ADRS + (x))); \
		EIEIO ; \
		(*val) = LONGSWAP(tmpVar); \
		EIEIO ; \
	}
    
#define MV64360_REG_WR(x,val)	\
	{	\
		UINT32 tmpVar ; \
                tmpVar = LONGSWAP(val); \
		EIEIO ; \
		*((volatile UINT32*)(MV64360_BASE_ADRS + (x))) = tmpVar ; \
		EIEIO ; \
	}
#endif
void usrAppInit (void)
{
    char shellPromp[20];
    BOOT_PARAMS       bootParams;

    FILE *fdMac;
    int len;    
    UINT32 gppRegVal;
    
    /**** add default route ****/
	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    if (bootParams.gad[0] != EOS)
	{
		if(routeAdd ("0.0.0.0", bootParams.gad) == ERROR)
		{
			printf("Unable to add route to %s; errno = 0x%x.\n", "0.0.0.0", errno);
		}
		/**/
#if 1
		{
		char bootDev[BOOT_DEV_LEN+1];
		UINT32 netMask = 0;
		struct in_addr host;
		struct in_addr gate;

		sprintf(bootDev, "%s%d", bootParams.bootDev, bootParams.unitNum);
		ifMaskGet(bootDev, &netMask);
		host.s_addr = inet_addr(bootParams.had);
		gate.s_addr = inet_addr(bootParams.gad);
        if ( (inet_netof(host) != inet_netof(gate))
			&&((inet_addr (bootParams.had) & netMask) == (inet_addr (bootParams.gad) & netMask)) )
        	{
        	/*
        	 *inet=192.168.2.*:ffff0000, host=192.168.3.*,gateway=192.168.2.1���������ɾ��·��"192.168.3.0 192.168.2.1"
        	 *inet=192.168.2.*, host=192.168.2.*,gateway=192.168.2.1�������������ɾ��·��
			 */
			char nad[20];
			inet_netof_string (bootParams.had, nad);
        	routeDelete(nad, bootParams.gad);
        	}
		else
			{
			}
		}
#endif
	}

    bspTextProtect();    /* protect text memory */

    bspSetSystemTime();    /* initialize and create RTC and set system time */

	rebootHookAdd(reset_BTS);

    StartWdtTask();

    taskPrioritySet(taskNameToId("tShell"), 200);  
    /**********************/


#ifdef	USER_APPL_INIT
	USER_APPL_INIT;		/* for backwards compatibility */
#endif
    enableNvramWrite();
    taskSuspend(taskNameToId("tShell"));
    sysNvDataModifyHandle();  /* change bts config parameter prompt */
    taskResume(taskNameToId("tShell"));    

	check_i2c_eeprom();

    /* add application specific code here */
    sprintf(shellPromp, "bts(0x%x) L3->", bspGetBtsID());
    shellPromptSet(shellPromp); 

    bspCreateRamDisk();

	spyLibInit ();
    if(ataDrvInstalled)
    {
            printErr("\nnow get boot para from CF,please wait.....\n");
            BtsMacGet(0, btsMac);    
            
            /*�����жϱ����վmac���ļ�jy080924*/
            if ((fdMac = fopen (cfMacFilename, "r")) == (FILE *)ERROR)/*����򲻿��Ͳ���cf����*/
            {
                printErr("\nCan't open %s file\n", cfMacFilename);
            }
            else
            {
                len=fread( ctemp, 1,6, fdMac);         
                fclose (fdMac);
                if(len == 0)/*�������Ϊ0��ʾ�״�ʹ��,��Ҫд���վmac*/
                {
                    printErr ("\n%s file length is 0\n", cfMacFilename); 
                    if((fdMac = fopen (cfMacFilename, "w+")) == (FILE *)ERROR)
                    {
                        printErr ("\nCannot open %s, when write after read error\n", cfMacFilename); 
                    }
                    else
                    {              
                        fwrite((const void *)btsMac, 1, 6, fdMac); 
                        fflush(fdMac);
                        fclose (fdMac);
                    }
                    getDataFromCF(l3FileName, (char*)BOOT_LINE_ADRS_L3,  L3BOOTLINEFILE, TRUE);
                    getDataFromCF(l2FileName, (char*)BOOT_LINE_ADRS_L2,L2BOOTLINEFILE, TRUE);
                    getDataFromCF(FileName, (char*)NVRAM_BASE_ADDR_APP_PARAMS,NVEMSDATA, TRUE);                    
                    getDataFromCF(btsFileName, (char*)NVRAM_BASE_ADDR_PARA_PARAMS, BTSPARA, TRUE) ;
                    
                    
                }
                else/*���Ȳ�Ϊ0��ʾ�����״�ʹ��,��Ҫ�ж�mac�Ƿ��ǻ�վ��*/
                {
                     if(memcmp(ctemp, btsMac, 6)==0)/*mac���,��ͨ����֤,������cf�������ļ�*/
                    {     
                        getDataFromCF(l3FileName, (char*)BOOT_LINE_ADRS_L3,  L3BOOTLINEFILE, FALSE);
                        getDataFromCF(l2FileName, (char*)BOOT_LINE_ADRS_L2,L2BOOTLINEFILE, FALSE);
                        getDataFromCF(FileName, (char*)NVRAM_BASE_ADDR_APP_PARAMS,NVEMSDATA, FALSE);
                        getDataFromCF(btsFileName, (char*)NVRAM_BASE_ADDR_PARA_PARAMS, BTSPARA, FALSE) ;
                        
                        
                    }
                     else
                    {
                        printErr("\nbts mac check error!used nvram data\n");
                    }
                }
            }
      
        }
#ifndef WBBU_CODE

/*��Դ���ƣ���909��Ŀ����ֲ����*/
    /*GPIO m909*/
    /*init*/
    MV64360_REG_RD(MV64360_MPP_CTRL0, &gppRegVal);   
    gppRegVal &= (0xFF00FFFF);                  
    MV64360_REG_WR(MV64360_MPP_CTRL0, gppRegVal);    

    MV64360_REG_RD(MV64360_GPP_IO_CTRL, &gppRegVal);   
    gppRegVal |= ((1 << 4) | (1 << 5));                  
    MV64360_REG_WR(MV64360_GPP_IO_CTRL, gppRegVal);    
    
    /*set pin 5 0*/
    MV64360_REG_RD(MV64360_GPP_VAL, &gppRegVal);    
    gppRegVal |= (1 << 4);  /***write 1***/
    gppRegVal &= ~(1 <<5); /***write 0***/
    MV64360_REG_WR(MV64360_GPP_VAL, gppRegVal);      
#endif
    
    if( ERROR == taskSpawn ("tAppRoot", APP_LOADER_TASK_PRIORITY, 0, 10240, (FUNCPTR)main, 0,0,0,0,0,0,0,0,0,0))
    {
        printf("task spawn main failed\n");
    }
}

void getDataFromCF(char *filename, char *nvaddr, int type, char first)
{
    FILE *fd;  
    char tmpbuf[BOOT_LINE_SIZE];
    int len,i,ztemp=0, result=0;
    int fileLen;    
    T_BootLoadState loaderParam; 
    char datanvramhdr[12];
    int ttemp;

    if((filename==NULL)||(nvaddr==NULL))
        return;
    /*printErr ("\ncome into getDataFromCF:%s \n", filename);*/
    if ((fd = fopen (filename, "r")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open %s when read.\n", filename); 
        if(first == TRUE)/*��һ��ʹ��cf��,�����nvram��Ч������crcд��cf*/
        {
            if((fd = fopen (filename, "w+")) == (FILE *)ERROR)
            {
                printErr ("\nCannot open %s when write.\n", filename); 
            }
            else
            {     
                if(type == NVEMSDATA)
                {                    
                    if(*((unsigned int *)(NVRAM_BASE_ADDR_APP_PARAMS)) == NVRAM_VALID_PATTERN)
                    {                        
                        cfGetCrc(nvaddr,sizeof(T_NVRAM_BTS_CONFIG_DATA));
                        fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_DATA)+6, fd); 
                    }                    
                }
                else if((type == L3BOOTLINEFILE)||(type == L2BOOTLINEFILE))
                {
                    if(*((unsigned int *)(NVRAM_BASE_ADDR_BOOT_STATE)) == NVRAM_VALID_PATTERN)
                    {                                                      
                            cfGetCrc(nvaddr,0);
                            fwrite( (const void *)nvaddr, 1,BOOT_LINE_SIZE, fd);                                 
                            
                    } 
                }     
                else if(type == BTSPARA)
                {
                    if(*((unsigned int *)(NVRAM_BASE_ADDR_PARA_PARAMS)) == NVRAM_VALID_PATTERN)
                    {                        
                        cfGetCrc(nvaddr,sizeof(T_NVRAM_BTS_CONFIG_PARA));
                        fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_PARA)+6, fd); 
                    }                    
                }
                fflush(fd);
                fclose (fd);
            }
        }
        else/*��Ҫ���nvram crc*/
        {
            if((fd = fopen (filename, "w+")) == (FILE *)ERROR)
            {
                printErr ("\nCannot open %s when write.\n", filename); 
            }
            else
            {
                if(type == NVEMSDATA)
                {                    
                    if((*((unsigned int *)(NVRAM_BASE_ADDR_APP_PARAMS)) == NVRAM_VALID_PATTERN)\
                        &&(cfCrc(nvaddr, sizeof(T_NVRAM_BTS_CONFIG_DATA))==OK))
                    {  
                        fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_DATA)+6, fd); 
                    }                    
                }
                else if(type == BTSPARA)
                {                    
                    if((*((unsigned int *)(NVRAM_BASE_ADDR_PARA_PARAMS)) == NVRAM_VALID_PATTERN)\
                        &&(cfCrc(nvaddr, sizeof(T_NVRAM_BTS_CONFIG_PARA))==OK))
                    {   
                        fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_PARA)+6, fd); 
                    }                    
                } 
                else if((type == L3BOOTLINEFILE)||(type == L2BOOTLINEFILE))                
                {
                   if( *((unsigned int *)(nvaddr+BOOT_LINE_SIZE - 7))== 0x5a5a5a5a)/*ʹ��c�����޸Ĺ�*/
                   {
                       bspNvRamWrite((char *)nvaddr+BOOT_LINE_SIZE - 7, (char*)&ztemp, 4);   
                       cfGetCrc(nvaddr,0);
                        fwrite( (const void *)nvaddr, 1,BOOT_LINE_SIZE, fd); 
                   }
                   else if((*((unsigned int *)(NVRAM_BASE_ADDR_BOOT_STATE)) == NVRAM_VALID_PATTERN)\
                        &&(cfCrc(nvaddr, 0)==OK))
                    {                               
                            fwrite((const void *) nvaddr, 1,BOOT_LINE_SIZE, fd);                                 
                            
                    }                     
                }                
                fflush(fd);
                fclose (fd);
            }
        }        
    }
    /*����򿪳ɹ�,������Ч,��д��nvram,�������nvram������Ч,д���ļ�*/
    else
    {         
        if((type == L3BOOTLINEFILE)||(type == L2BOOTLINEFILE))                
        {            
           if( *((unsigned int *)(nvaddr+BOOT_LINE_SIZE - 7))== 0x5a5a5a5a)/*ʹ��c�����޸Ĺ�*/
           {
               bspNvRamWrite((char *)nvaddr+BOOT_LINE_SIZE - 7, (char*)&ztemp, 4);   
               cfGetCrc(nvaddr,0);               
               fclose (fd);
               if((fd = fopen (filename, "w+")) == (FILE *)ERROR)
               {
                   printErr ("\nCannot open %s  .\n", filename); 
               }
               else   
               {
                   fwrite( (const void *)nvaddr, 1,BOOT_LINE_SIZE, fd); 
                   fclose (fd); 
               }
               return;
           }
           
        }
        fileLen = fread( tmpbuf, 1,BOOT_LINE_SIZE, fd);   
        if(type == NVEMSDATA)
            len = sizeof(T_NVRAM_BTS_CONFIG_DATA);
        else if(type == BTSPARA)
            len = sizeof(T_NVRAM_BTS_CONFIG_PARA);
        else
            len = strlen(tmpbuf);
        if((fileLen>0)&& (cfCrc(tmpbuf, len)==OK))/*���У��ͨ����д��nvram*/
        { 
            /*printErr("\n%s pass crc check \n", filename); */
            fclose (fd);    
            /*���cf���ݺ�nvramһ�¾������ⲿ��*/
            if((type == NVEMSDATA)||(type == BTSPARA))
                result = memcmp(tmpbuf, nvaddr, len); 
            else
                result = strcmp(tmpbuf, (char *)nvaddr);
            if(result==0)
            {
                printErr("\n%s:  cf=nvram\n", filename);
            }
            else/*д��nvram,ͬʱ��λ��վ*/
            {                 
                if(type==L3BOOTLINEFILE)/*ems������ݲ���Ҫ�����վ*/
                {              
                    if(*((unsigned int *)(NVRAM_BASE_ADDR_BOOT_STATE)) != NVRAM_VALID_PATTERN)
                    {    
                        /* if the pattern is wrong, reinitializa */
                        memset(&loaderParam, 0, sizeof(T_BootLoadState));
                        loaderParam.workFlag = LOAD_STATUS_VERIFIED;
                        loaderParam.bootPlane = BOOT_PLANE_A;
                        loaderParam.nvramSafe = NVRAM_VALID_PATTERN;   
                        bspNvRamWrite((char *)(NVRAM_BASE_ADDR_BOOT_STATE), (char *)&loaderParam, sizeof(T_BootLoadState)); 
			   printErr("\nreset NVRAM_BASE_ADDR_BOOT_STATE in usrAppInit\n"); 
			   memset(datanvramhdr, 0, 12);
			   bspNvRamWrite((char *)NVRAM_BASE_ADDR_DATA, datanvramhdr, 12);
			   ttemp = 0;
			   bspNvRamWrite((char *)NVRAM_BASE_ADDR_OAM, (char*)&ttemp, 4);
                    }
                    printErr("\nreboot bts, pls wait...\n"); 
                    bspNvRamWrite((char *)nvaddr, tmpbuf, (len+6));  		             
		 
                    bspSetBtsResetReason(RESET_REASON_L3BOOTLINE_DIFF);
                    
                    rebootBTS(0);                
                }
                else if(type == L2BOOTLINEFILE)
                {
                    bspNvRamWrite((char *)nvaddr, tmpbuf, (len+6)); 
                }
                else if(type == NVEMSDATA)
                    bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, tmpbuf, (len+6));
                else if(type == BTSPARA)
                    bspNvRamWrite((char *)NVRAM_BASE_ADDR_PARA_PARAMS, tmpbuf, (len+6));                    
                
                }                
        }/*�������nvram����У��ͨ����д��cf��*/
        else
        {
            fclose (fd);
            if((fd = fopen (filename, "w+")) == (FILE *)ERROR)
            {
                printErr ("\nCannot open %s when crc error .\n", filename); 
            }
            else                
            {
                if(first == TRUE)/*�����һ��ʹ��,��У��crc*/
                {
                    if(type == NVEMSDATA)
                    {
                        if(*((unsigned int *)(NVRAM_BASE_ADDR_APP_PARAMS)) == NVRAM_VALID_PATTERN)
                        {
                            cfGetCrc(nvaddr,sizeof(T_NVRAM_BTS_CONFIG_DATA));                            
                            fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_DATA)+6, fd); 
                                                
                        }
                    }
                    else if(type == BTSPARA)
                    {
                        if(*((unsigned int *)(NVRAM_BASE_ADDR_PARA_PARAMS)) == NVRAM_VALID_PATTERN)
                        {
                            cfGetCrc(nvaddr,sizeof(T_NVRAM_BTS_CONFIG_PARA));
                            fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_PARA)+6, fd); 
                                                
                        }
                    }
                    else if((type == L3BOOTLINEFILE)||(type == L2BOOTLINEFILE)) 
                    {
                        if(*((unsigned int *)(NVRAM_BASE_ADDR_BOOT_STATE)) == NVRAM_VALID_PATTERN)
                        {
                            cfGetCrc(nvaddr,0);
                            fwrite((const void *)nvaddr, 1,BOOT_LINE_SIZE, fd);                                                  
                        }
                    }
                    fflush(fd);
                    fclose (fd);    
                }
                else/*У��crc*/
                {                   
                    if(type == NVEMSDATA)
                    {                        
                        if((*((unsigned int *)(NVRAM_BASE_ADDR_APP_PARAMS)) == NVRAM_VALID_PATTERN)\
                            &&(cfCrc(nvaddr, sizeof(T_NVRAM_BTS_CONFIG_DATA))==OK))
                        {                             
                            fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_DATA)+6, fd); 
                        }
                    }
                    else if(type == BTSPARA)
                    {
                        if((*((unsigned int *)(NVRAM_BASE_ADDR_PARA_PARAMS)) == NVRAM_VALID_PATTERN)\
                            &&(cfCrc(nvaddr, sizeof(T_NVRAM_BTS_CONFIG_PARA))==OK))
                        {                            
                            fwrite( (const void *)nvaddr, 1,sizeof(T_NVRAM_BTS_CONFIG_PARA)+6, fd); 
                                                
                        }
                    }
                    else if((type == L3BOOTLINEFILE)||(type == L2BOOTLINEFILE))                
                    {
                        if((*((unsigned int *)(NVRAM_BASE_ADDR_BOOT_STATE)) == NVRAM_VALID_PATTERN)\
                            &&(cfCrc(nvaddr, 0)==OK))
                        {                                                       
                                fwrite( (const void *)nvaddr, 1,BOOT_LINE_SIZE, fd);                                 
                                
                        }                    
                    }                
                    fflush(fd);
                    fclose (fd);
                }                
                    
            }
            
        }
        
    }
}
char BtsMacGet (  int unit, UINT8 *adrs)
{
    unsigned int ipAddr;	
    int byte;
    T_I2C_TABLE i2cData;

#if 0
	bcopy(bspEnetAdrs[unit], adrs, 5);
    ipAddr = GetBtsIpAddr();
    adrs[5] = ipAddr&0xff;
#else /***form i2c****/

/****mac i2c addr: 1010110 ******/
    i2c_read(I2C_E2PROM_DEV_ADDR, 0, &i2cData, sizeof(i2cData));
    bcopy(i2cData.L3_mac, adrs, 6);
#endif

    /* Check to see if mac address is valid. */
    for (byte=0; byte < 6; byte++)
        {
        if (adrs[byte] != 0xff)
            break;
        }
    
    
    
    if (byte == 6)
        {
        /* MAC address has ff:ff:ff:ff:ff:ff, copy default
         * address
         */
        logMsg("Using default MAC address for the unit mv%d\n",unit,1,2,3,4,5);
        bcopy(defaultBtsAdrs[unit], adrs, 6);
        }
   
    return(OK);
}



