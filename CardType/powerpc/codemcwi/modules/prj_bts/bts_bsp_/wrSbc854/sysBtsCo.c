/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:        sysBtsConfigData.c
 *
 * DESCRIPTION:   functions used to setup bts config parameters
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   04/04/2006   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#include <ioLib.h>
#include <fiolib.h>
#include "netinet/in.h"
#include "sysBtsConfigData.h"

#define isascii(c) ((unsigned )(c)<=0177)
#ifndef DEC
    #define DEC		FALSE	/* getArg parameters */
#endif

#ifndef HEX
    #define HEX		TRUE
#endif

#ifndef OPT 
    #define OPT		TRUE
#endif

#define MAX_ADR_SIZE 	6 
#define MAX_LINE        160	

#define MinutesToTicks(expr)  (6000 * (expr))

void sysNvDataModifyTask(char key)  ;
void sysBootConfigModify();
void sysNvDataModify();
extern int i2c_read(unsigned int device_addr, 
unsigned int offset_addr,
unsigned char* val, 
unsigned int len);
extern int i2c_write(unsigned int device_addr, 
unsigned int offset_addr,
unsigned char* val, 
unsigned int len);
/*因为编译问题,所以写了2遍函数jy080920*/
char cfCrc(char *pBuf, UINT16 len);
char cfGetCrc(char *pBuf, UINT16 len);
void  setboolineFlag();
/*****************************************************************************
 *
 *   Function:    sysNvRamDsValidationCheck()
 *
 *   Description: This function provides basic coherency checking
 *                of all existing parameters in the NV data store.
 *
 *                If incoherenent, a message is displayed an <ERROR>
 *                returned.
 *
 *   Parameters:  None
 *
 *   Returns   :  <OK>    - All parameters meet basic coherency checks
 *                <ERROR> - At least one parameter coherency is invalid
 *
 *   Caveats   :  None
 *
 *****************************************************************************
 */
STATUS sysNvRamDsValidationCheck()
{

    T_NVRAM_BTS_CONFIG_DATA  param;
    T_NVRAM_BTS_CONFIG_PARA param1;
    
    bspNvRamRead((char *)&param1, (char*)(NVRAM_BASE_ADDR_PARA_PARAMS), sizeof(param1));
    bspNvRamRead((char *)&param, (char*)(NVRAM_BASE_ADDR_APP_PARAMS), sizeof(param));

    if ( NVRAM_VALID_PATTERN != param.nvramSafe )
    {
/*<-----*/ 
        return(ERROR);
    }


    if ( BTS_BOOT_DATA_SOURCE_BTS != param.dataSource && BTS_BOOT_DATA_SOURCE_EMS!= param.dataSource )
    {
/*<-----*/ 
        return(ERROR);
    }

    if ( param.vlanId > MAX_VLAN_ID )
    {
/*<-----*/ 
        return(ERROR);
    }

    if ( param.btsRcvPort > 20000 || param.btsRcvPort < 1000 )
    {
        return(ERROR);
    }
    if ( param.emsRcvPort > 20000 || param.emsRcvPort < 1000 )
    {
        return(ERROR);
    }

    /* Make sure to setup/load real-time clock from NV data store */
    if ( NVRAM_VALID_PATTERN != param.rebootBtsWanIfDiscPattern)
    {
        param.rebootBtsWanIfDiscPattern = NVRAM_VALID_PATTERN;
        param.rebootBtsWanIfDisc = 0;
    }

    if ( NVRAM_VALID_PATTERN != param.permitPPPoEPattern)
    {
        param.permitPPPoEPattern = NVRAM_VALID_PATTERN;
        param.permitPPPoE = 1;
    }

    if ( NVRAM_VALID_PATTERN != param.filterSTPPattern)
    {
        param.filterSTPPattern = NVRAM_VALID_PATTERN;
        param.filterSTP = 1;
    }
    /*对新加的判断参数进行有效性校验*/
    
   
    if ( NVRAM_VALID_PATTERN != param1.rebootBtsIfFtpDownPattern)
    {
        param1.rebootBtsIfFtpDownPattern = NVRAM_VALID_PATTERN;
        param1.rebootBtsIfFtpDown= 0;
    }
    if ( NVRAM_VALID_PATTERN != param1.permitUseWhenSagDownPattern)
    {
        param1.permitUseWhenSagDownPattern = NVRAM_VALID_PATTERN;
        param1.permitUseWhenSagDown= 1;
    }
    return(OK);
}



static SEM_ID sysDataConfigSem;

void sysNvDataConfigWait(void)
{
    if ( 0 != sysDataConfigSem )
    {
        semTake(sysDataConfigSem, WAIT_FOREVER);
    }
}

void sysNvDataConfigPrep(void)
{
    sysDataConfigSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
}

void sysNvDataConfigFinishAnnounce(void)
{
    if ( 0 != sysDataConfigSem )
    {
        semGive(sysDataConfigSem);
    }

    while ( 1 )
    {
        taskDelay(MinutesToTicks(1));
    }
}

void sysNvDataConfigAnnounceClean(void)
{
    taskLock();

    if ( 0 != sysDataConfigSem )
    {
        semDelete(sysDataConfigSem);
        sysDataConfigSem = 0;
    }

    taskUnlock();
}

void sysNvDataShow()
{
#if 1
    T_NVRAM_BTS_CONFIG_DATA  params;
    T_NVRAM_BTS_CONFIG_PARA params1;
    T_NVRAM_BTS_NETWORK_PARA para;
    char ipStr[40];
    T_TimeDate dateTime;
    struct in_addr ipAddr;

    bspNvRamRead((char *)&params, (char*)(NVRAM_BASE_ADDR_APP_PARAMS), sizeof(params));
    bspNvRamRead((char *)&params1, (char*)(NVRAM_BASE_ADDR_PARA_PARAMS), sizeof(params1));
    bspNvRamRead((char *)&para, (char*)(NVRAM_BASE_ADDR_NETWORK_PARAS), sizeof(para));
    printf("\n");
    printf("btsId:            :%x\n", params.btsId); 
    printf("vlanID:           :%d\n", params.vlanId); 
    printf("dataSource        :%s\n", (params.dataSource ==BTS_BOOT_DATA_SOURCE_BTS)?"bts":"ems");
    ipAddr.s_addr = params.emsIp;
    inet_ntoa_b(ipAddr, ipStr);
    printf("main EMS IP       :%s\n", ipStr);
    printf("main EMS RX port  :%d\n", params.emsRcvPort);

    if (0 != params.bakemsIP)
        {
        ipAddr.s_addr = params.bakemsIP;
        inet_ntoa_b(ipAddr, ipStr);
        printf("backup EMS IP     :%s\n", ipStr);
        printf("backup EMS RX port:%d\n", params.bakemsRcvPort);
        }

    printf("BTS RX port       :%d\n\n", params.btsRcvPort);

    ipAddr.s_addr = params.btsIp;
    inet_ntoa_b(ipAddr, ipStr);
    printf("BTS public IP     :%s\n", ipStr);
    printf("BTS public port   :%d\n", params.btsPort);
    printf("Reboot BTS when WAN IF disconnected: ");
    if ( NVRAM_VALID_PATTERN != params.rebootBtsWanIfDiscPattern
        || 0 == params.rebootBtsWanIfDisc)
    {
        printf("%s\n", "no");
    }
    else
    {
        printf("%s\n", "yes");
    }

    printf("PPPoE packet forward: ");
    if ( NVRAM_VALID_PATTERN != params.permitPPPoEPattern
        || 0 == params.permitPPPoE)
    {
        printf("%s\n", "no");
    }
    else
    {
        printf("%s\n", "yes");
    }

    printf("Filter STP packet: ");
    if ( NVRAM_VALID_PATTERN != params.filterSTPPattern
        || 0 == params.filterSTP)
    {
        printf("%s\n", "no");
    }
    else
    {
        printf("%s\n", "yes");
    }
    printf("Reboot BTS when FTP is down:");
    if ( NVRAM_VALID_PATTERN != params1.rebootBtsIfFtpDownPattern
        || 1 == params1.rebootBtsIfFtpDown)
    {
        printf("%s\n", "yes");
    }
    else
    {
        printf("%s\n", "no");
    }
    #if 0
   printf(" BTS NetWork Para:");
    if ( NVRAM_VALID_PATTERN != para.BtsIfNETWORKPattern)
    {
        printf("BTS NetWork Para Is default:100M Auto\n");
		
    }
   else
   {
        if(para.TYPE == 0)
        {
           printf("BTS Network is AUTO\n");
        }
	  else
	  {
	    printf("BTS Network is NO_AUTO\n");
	  }
	    if(para.SPEED == 0)
        {
           printf("BTS Network SPEED is 10M\n");
        }
	  else
	  {
	    printf("BTS Network SPEED is 100M\n");
	  }

	     if(para.mode == 0)
        {
           printf("BTS Network is half duplex\n");
        }
	  else
	  {
	    printf("BTS Network is full duplex\n");
	  }
   }
   #endif
    printf("Permit use when SAG is down:");
     if ( NVRAM_VALID_PATTERN != params1.permitUseWhenSagDownPattern
        || 1 == params1.permitUseWhenSagDown)
    {
        printf("%s\n", "yes");
    }
    else
    {
        printf("%s\n", "no");
    }

      printf("RRU Channel:");
     if ( 4== params1.RRU_Antenna)
    {
        printf("4 Channels\n");
    }
    else /*if(8== params1.RRU_Antenna)*/
    {
        printf("8 Channels\n");
    }
    //zengjihan 20120801 for GPSSYNC
    if( NVRAM_VALID_PATTERN != params1.slaveBTSIDFLAG)    
    {
        printf("slaveBTSID  :%x\n", 0);
    }
    else
    {
        printf("slaveBTSID  :%x\n", params1.slaveBTSID);
    }

    dateTime = bspGetDateTime();
    printf("date/time         :[%d/%d/%d %d:%d:%d]\n\n", dateTime.month, dateTime.day, dateTime.year,dateTime.hour, dateTime.minute, dateTime.second);
#endif
}


/*****************************************************************************
 *
 *   Function:    sysSetBtsConfigData()
 *
 *   Description: This function provides a method to countdown an autoboot
 *                sequence. A any key is pressed during the countdown period.
 *                
 *
 *   Parameters:  None
 *
 *   Returns   :  Nothing
 *
 *   Caveats   :  This function is copied from windriver's autoboot()
 *                in their boot rom
 *
 *****************************************************************************
 */
static BOOL isConfigChanged;
#define BTS_CONFIG_TIMEOUT  3
#define BTS_CONFIG_KEY_MATCH_TIME   20
void sysNvDataModifyHandle(void)
{
    int     timeout = 0;
    ULONG   autoBootTime;
    int     timeLeft;
    UINT    timeMarker;
    int     bytesRead = 0;
//    FUNCPTR entry;
    char    key = 0;
    UINT32  oldOpts;
    int     consoleInFd;
    BOOL    needToConfig = FALSE;
    int tConfigModTid = 0;


    consoleInFd = ioGlobalStdGet(STD_IN);

    timeout = BTS_CONFIG_TIMEOUT;

    if ( ERROR == sysNvRamDsValidationCheck() )
    {
        printf("BTS Config Data in NVRAM is not valid, need to modify\n");
        /*needToConfig = TRUE;*//*可以不中止启动,cf卡会导入的jy090226*/
    }
  /*  else*/
    {
        if ( timeout > 0 )
        {
            printf("\n\nMcWill L3 booting (Enter \"mcwill\" to config)...  ");

            /* Loop looking for a char, or timeout after specified seconds */
            autoBootTime = tickGet () + sysClkRateGet () * timeout;

            timeMarker = tickGet () + sysClkRateGet ();
            timeLeft = timeout;

            printf ("%2d", timeLeft);
            oldOpts = ioctl(consoleInFd, FIOGETOPTIONS, 0); 
            ioctl (consoleInFd, FIOSETOPTIONS, OPT_RAW | OPT_ECHO | OPT_CRMOD | OPT_TANDEM | OPT_7_BIT ); 

            while ( (tickGet () < autoBootTime) && (bytesRead == 0) )
            {
                ioctl (consoleInFd, FIONREAD, (int) &bytesRead);

                if ( tickGet () == timeMarker )
                {
                    timeMarker = tickGet () + sysClkRateGet ();
                    printf ("\b\b%2d", --timeLeft);
                }
                taskDelay(1); /* Yield for a momement */
            }
        }
        /* read the key that stopped autoboot */

        if ( 0 != bytesRead )    /* nothing typed so auto-boot */
        {
            needToConfig = TRUE;
            read (consoleInFd, &key, 1);
        }

        (void) ioctl (consoleInFd, FIOSETOPTIONS, oldOpts);
    }

    isConfigChanged = FALSE;
    if ( needToConfig )
    {
        sysNvDataConfigPrep();
        tConfigModTid = taskSpawn ("tBtCfg", 50, 0, 10240, 
                        (FUNCPTR)sysNvDataModifyTask, key,
                        0,0,0,0,0,0,0,0,0);

        sysNvDataConfigWait();
        tConfigModTid && taskDelete(tConfigModTid);
        sysNvDataConfigAnnounceClean();

    }

    printf("\r L3 Application booting.............................. Done\n");
    printf("\n");
}


LOCAL void bspNvConfigHelp (void)
{
    static char *helpMsg[] =
    {
        "?",                      "- print this list",
        "@",                      "- resume boot sequence",
        "p",                      "- print BTS NVRam config data",
        "c",                      "- change BTS NVRAM config data",
        "b",                      "- change BTS boot control data",
        "d adrs[,n]",             "- display memory",
        "m adrs",                 "- modify memory",
        NULL
    };
    char **pMsg;

    printf("\n");
    for ( pMsg = helpMsg; *pMsg != NULL; pMsg += 2 )
    {
        printf (" %-21s %s\n", *pMsg, *(pMsg + 1));
    }
    printf("\n");

}
LOCAL void printClear1
    (
    FAST char *param
    )
    {
    FAST char ch;

    while ((ch = *(param++)) != EOS)
	printf ("%c", (isascii (ch) && isprint (ch)) ? ch : '?');
    }
LOCAL int promptRead1
    (
    char *buf,
    int bufLen
    )
    {
    FAST int i;

    i = fioRdString (STD_IN, buf, bufLen);

    if (i == EOF)
	return (-99);			/* EOF; quit */

    if (i == 1)
	return (1);			/* just CR; leave field unchanged */

    if ((i == 2) && (buf[0] == '-'))
	return (-1);			/* '-'; go back up */

    if (i >= bufLen)
	{
	printf ("too big - maximum field width = %d.\n", bufLen);
	/* We mustn't take into account the end of the string */ 
	while((i = fioRdString(STD_IN, buf, bufLen)) >= bufLen);
	return (-98);
	}

    return (0);
    }
#define PARAM_PRINT_WIDTH	21
int promptParamNum1
    (
    char *paramName,
    int *pValue,
    BOOL hex
    )
    {
    char buf [BOOT_FIELD_LEN];
    char *pBuf;
    int value;
    int i;

    printf (hex ? "%-*s: 0x%x " : "%-*s: %d ", PARAM_PRINT_WIDTH, paramName,
	    *pValue);

    if ((i = promptRead (buf, sizeof (buf))) != 0)
	return (i);

    if (buf[0] == '.')
	{
	*pValue = 0;		/* just '.'; make empty field (0) */
	return (1);
	}

    /* scan for number */

    pBuf = buf;
    if ((bootScanNum (&pBuf, &value, FALSE) != OK) || (*pBuf != EOS))
	{
	printf ("invalid number.\n");
	return (-98);
	}

    *pValue = value;
    return (1);
    }
LOCAL void skipSpace1
    (
    FAST char **strptr  /* pointer to pointer to string */
    )
    {
    while (isspace (**strptr))
	++*strptr;
    }
LOCAL int promptParamString1
    (
    char *paramName,
    FAST char *param,
    int sizeParamName
    )
    {
    FAST int i;
    char buf [BOOT_FIELD_LEN];

    printf ("%-*s: ", PARAM_PRINT_WIDTH, paramName);
    printClear (param);
    if (*param != EOS)
	printf (" ");

    if ((i = promptRead (buf, sizeParamName)) != 0)
	return (i);

    if (buf[0] == '.')
	{
	param [0] = EOS;	/* just '.'; make empty field */
	return (1);
	}

    strcpy (param, buf);	/* update parameter */
    return (1);
    }
/******************************************************************************
*
* getArg - get argument from command line
*
* This routine gets the next numerical argument from the command line.
* If the argument is not optional, then an error is reported if no argument
* is found.  <ppString> will be updated to point to the new position in the
* command line.
* copied from bootConfig.c
* RETURNS: OK or ERROR
*/

LOCAL STATUS getArg1
(
FAST char **ppString,   /* ptr to ptr to current position in line */
int *   pValue,     /* ptr where to return value */
BOOL    defaultHex, /* TRUE = arg is hex (even w/o 0x) */
BOOL    optional    /* TRUE = ok if end of line */
)
{
    skipSpace (ppString);


    /* if nothing left, complain if arg is not optional */

    if ( **ppString == EOS )
    {
        if ( !optional )
        {
/*            printf ("missing parameter\n"); */
            return(ERROR);
        }
        else
            return(OK);
    }


    /* scan arg */

    if ( bootScanNum (ppString, pValue, defaultHex) != OK )
    {
        printf ("invalid parameter\n");
        return(ERROR);
    }

    skipSpace (ppString);

    /* if we encountered ',' delimiter, step over it */

    if ( **ppString == ',' )
    {
        ++*ppString;
        return(OK);
    }

    /* if end of line, scan is ok */

    if ( **ppString == EOS )
        return(OK);

    /* we got stopped by something else */

    printf ("invalid parameter\n");
    return(ERROR);
}
/******************************************************************************
*
* bootSubfieldExtract - extract a numeric subfield from a boot field
*
* Extracts subfields in fields of the form "<field><delimeter><subfield>".
* i.e. <inet>:<netmask> and bp=<anchor>
*/

LOCAL STATUS bootSubfieldExtract1
    (
    char *string,       /* string containing field to be extracted */
    int *pValue,        /* pointer where to return value */
    char delimeter      /* character delimeter */
    )
    {
    FAST char *pDelim;
    int value;

    /* find delimeter */

    pDelim = index (string, delimeter);
    if (pDelim == NULL)
	return (0);		/* no subfield specified */

    /* scan remainder for numeric subfield */

    string = pDelim + 1;

    if (bootScanNum (&string, &value, TRUE) != OK)
	return (-1);		/* invalid subfield specified */

    *pDelim = EOS;		/* terminate string at the delimeter */
    *pValue = value;		/* return value */
    return (1);			/* valid subfield specified */
    }
STATUS bootNetmaskExtract2
    (
    char *string,       /* string containing addr field */
    int *pNetmask       /* pointer where to return net mask */
    )
    {
    FAST char *pDelim;
    char *offset;

    /* find delimeter */

    pDelim = index (string, ':');
    if (pDelim == NULL)
	return (0);		/* no subfield specified */

    /* Check if netmask field precedes timeout field. */
    offset = pDelim + 1;
    skipSpace(&offset);
    if (*offset == ':' || *offset == EOS)  /* Netmask field is placeholder. */
        {
         *pDelim = EOS;
         *pNetmask = 0;
         return (1);
        }
         
    return (bootSubfieldExtract (string, pNetmask, ':'));
    }
LOCAL STATUS checkInetAddrField1 
    (
    char *pInetAddr,
    BOOL subnetMaskOK
    )
    {
    char inetAddr [30];
    int netmask;

    /* 
     * The bzero() call corrects SPR 6326. The calls to bootNetmaskExtract()
     * and inet_addr() did not delimit the input string with a '\0'. When
     * inet_addr attempted to print the invalid address, the system would
     * crash or hang.
     */

    bzero (inetAddr, sizeof(inetAddr));

    if (*pInetAddr == EOS)
	return (OK);

    strncpy (inetAddr, pInetAddr, sizeof (inetAddr) - 1);

    if (subnetMaskOK)
	{
	if (bootNetmaskExtract1 (inetAddr, &netmask) < 0)
	    {
	    printf ("Error: invalid netmask in boot field \"%s\".\n", inetAddr);
	    return (ERROR);
	    }
	}

    if (inet_addr (inetAddr) == (ULONG) ERROR)
	{
	printf ("Error: invalid inet address in boot field \"%s\".\n",inetAddr);
	return (ERROR);
	}

    return (OK);
    }

/*****************************************************************************
 *
 *   Function:    sysNvDataModify()  
 *
 *   Description: This function provides a boot line menu command prompt.
 *
 *   Parameters:  None
 *
 *   Returns   :  Nothing
 *
 *   Caveats   :  This function is copied from windriver's boot rom
 *
 *****************************************************************************
 */
void sysNvDataModifyTask(char key)  
{
    char line [160];
    char *pLine;
    int  consoleInFd;
    int adr;
    int adr2;
    int nwords;

    unsigned int autoBootTime;
    static const char* match = "mcwill";
    unsigned int matchIndex = 0;
    unsigned int bytesRead = 1; /* From input <key> */
    unsigned int oldOpts;

    consoleInFd = ioGlobalStdGet(STD_IN);
    oldOpts = ioctl(consoleInFd, FIOGETOPTIONS, 0);

    if ( key != 0 )  /* get in because of key pressing, not because of invalid NVRAM data */
    {
        ioctl (consoleInFd, FIOSETOPTIONS, OPT_RAW|OPT_7_BIT|OPT_ECHO | OPT_CRMOD );

        autoBootTime = tickGet () + sysClkRateGet () * BTS_CONFIG_KEY_MATCH_TIME; /* seconds */
        do
        {
            if ( (  (bytesRead > 0) && (match[matchIndex] != key)) 
            || (tickGet() >= autoBootTime)
            )
            {
                ioctl (consoleInFd, FIOSETOPTIONS, oldOpts);
                sysNvDataConfigFinishAnnounce();
                while ( 1 ) taskDelay(MinutesToTicks(1));
                /*<----------*/ return;
            }

            if ( bytesRead > 0 )
            {
                matchIndex++;
            }

            taskDelay(1); /* Yield for a momement */

            (void) ioctl (consoleInFd, FIONREAD, (int) &bytesRead);

            if ( bytesRead > 0 )
            {
                read (consoleInFd, &key, 1);

                /* Discard all non-CR control characters */
                if ( (key < ' ')
                && (key != '\n')
                )
                {
                    bytesRead = 0;
                }
            }
        } while ( matchIndex < strlen(match) );
    }

    key = ' ';

    printf("\n c -config, p - display, b -boot parameter, h/? - help, @ boot\n");

    FOREVER
    {
        if ( (key == '!') || (key == '@') )
        {
            line [0] = key;
            line [1] = EOS;
            key = 0;
        }
        else
        {
            printf ("[Mcwill BOOT]: ");
            fioRdString (STD_IN, line, sizeof (line));
        }

        adr = adr2 = 0;
        nwords = 0;

        /* take blanks off end of line */
        pLine = line + strlen (line) - 1;       /* point at last char */
        while ( (pLine >= line) && (*pLine == ' ') )
        {
            *pLine = EOS;
            pLine--;
        }

        pLine = line;
        skipSpace (&pLine);

        switch ( *(pLine++) )
        {
            case EOS:       /* blank line */
                break;

            case 'd':       /* display */
                if ( (getArg (&pLine, &adr, HEX, OPT) == OK) &&
                (getArg (&pLine, &nwords, DEC, OPT) == OK) )
                    d ((char *) adr, nwords, 2);
                break;

            case 'm':       /* modify */
                if ( getArg (&pLine, &adr, HEX, !OPT) == OK )
                    m ((char *) adr, 2);
                break;

            case 'p':       /* print boot params */
                sysNvDataShow();
                break;

            case 'c':       /* change bts config params */
                sysNvDataModify();
                break;

            case 'b':      /* change boot parameters */
                sysBootConfigModify();
                break;

            case '?':           /* help */
            case 'h':           /* help */
                bspNvConfigHelp ();
                break;

            case '@':           /* load and go with internal params */
            case '$':           /* load and go with internal params */
                isConfigChanged = FALSE;
                (void) ioctl (consoleInFd, FIOSETOPTIONS, oldOpts);
                sysNvDataConfigFinishAnnounce();
                while ( 1 ) taskDelay(MinutesToTicks(1));
                /*<-----*/  return;

            default:
                printf ("Unrecognized command. Type '?' for help.\n");
                break;

        } /* switch */
    } /* FOREVER */
}



/*******/
void sysNvDataModify()
{

    T_NVRAM_BTS_CONFIG_DATA   params;
    T_NVRAM_BTS_CONFIG_PARA params1;
    int n = 0;
    int i;
    char emsip[20],bakemsip[20];
 //   char macaddr[20];
    struct in_addr emsIpInt; 
    unsigned int tmpValue;
    T_TimeDate dateTime;
       struct tm time_s;
  //  T_TimeDate timeDate;
     unsigned int secCount;
        struct timespec timeSpec;
    char FileName[20] = "/ata0a/nvEmsData";    
    char FileName1[20] = "/ata0a/btsPara";    
    FILE *fd;
//    unsigned char memtmp[256];
   char pbuffer[7];
    bspNvRamRead((char *)&params, (char*)(NVRAM_BASE_ADDR_APP_PARAMS), sizeof(params));
    bspNvRamRead((char *)&params1, (char*)(NVRAM_BASE_ADDR_PARA_PARAMS), sizeof(params1));
    if ( params.nvramSafe != NVRAM_VALID_PATTERN )
    {
        memset(&params, 0, sizeof(params));
        params.nvramSafe = NVRAM_VALID_PATTERN;
        params.dataSource = BTS_BOOT_DATA_SOURCE_EMS;
        params.emsRcvPort = DEFAULT_EMS_RCV_PORT;
        params.btsRcvPort = DEFAULT_BTS_RCV_PORT;
        params.rebootBtsWanIfDiscPattern = NVRAM_VALID_PATTERN;
        params.rebootBtsWanIfDisc = 0;
        params.permitPPPoEPattern = NVRAM_VALID_PATTERN;
        params.permitPPPoE = 1;
        params.filterSTPPattern = NVRAM_VALID_PATTERN;
        params.filterSTP = 1;
        params1.rebootBtsIfFtpDownPattern = NVRAM_VALID_PATTERN;
        params1.rebootBtsIfFtpDown = 0;
        params1.permitUseWhenSagDownPattern = NVRAM_VALID_PATTERN;
        params1.permitUseWhenSagDown= 1;
    }
    if (NVRAM_VALID_PATTERN != params.rebootBtsWanIfDiscPattern)
    {
        params.rebootBtsWanIfDiscPattern = NVRAM_VALID_PATTERN;
        params.rebootBtsWanIfDisc = 0;
    }

    if (NVRAM_VALID_PATTERN != params.permitPPPoEPattern)
    {
        params.permitPPPoEPattern = NVRAM_VALID_PATTERN;
        params.permitPPPoE = 1;
    }

    if (NVRAM_VALID_PATTERN != params.filterSTPPattern)
    {
        params.filterSTPPattern = NVRAM_VALID_PATTERN;
        params.filterSTP = 1;
    }
    if (NVRAM_VALID_PATTERN != params1.rebootBtsIfFtpDownPattern)
    {
        params1.rebootBtsIfFtpDownPattern = NVRAM_VALID_PATTERN;
        params1.rebootBtsIfFtpDown= 0;
    }
    if (NVRAM_VALID_PATTERN != params1.permitUseWhenSagDownPattern)
    {
        params1.permitUseWhenSagDownPattern = NVRAM_VALID_PATTERN;
        params1.permitUseWhenSagDown= 1;
    	}
    if((params1.RRU_Antenna!=4)&&(params1.RRU_Antenna!=8))
    	{
    	    params1.RRU_Antenna = 8;/*默认为8通道*/
    	}
    //zengjihan 20120801 for GPSSYNC
    if(NVRAM_VALID_PATTERN != params1.slaveBTSIDFLAG)
    {
        params1.slaveBTSID= 0; 
    }
    dateTime = bspGetDateTime();
    if ( dateTime.month > 12 || dateTime.day > 31 || dateTime.hour >23 || dateTime.minute > 59 || dateTime.second >59 )
    {
        dateTime.year = 2006;
        dateTime.month = 1;
        dateTime.day = 1;
        dateTime.hour = 0;
        dateTime.minute = 0;
        dateTime.second = 0;
    }


    emsIpInt.s_addr = params.emsIp;
    inet_ntoa_b(emsIpInt, emsip);
    emsIpInt.s_addr = params.bakemsIP;
    inet_ntoa_b(emsIpInt, bakemsip);

    printf ("\n'.' = clear field;  '-' = go to previous field;  ^D = quit\n\n");

    /* prompt the user for each item;
     *   i:  0 = same field, 1 = next field, -1 = previous field,
     *     -98 = error, -99 = quit
     */

    FOREVER
    {
        switch ( n )
        {
            case 0:
                i = promptParamNum ("btsId: If input hex number, please add 0x!!!The system defaults decimal number!", &(params.btsId), TRUE);
                printf("\n");
                break;
            case 1:  
                i = promptParamNum ("vlanId (0~4095)", &(params.vlanId), FALSE); 
                if ( params.vlanId >= MAX_VLAN_ID )
                {
                    printf("value too big\n");
                    i = 0;
                }
                printf("\n");
                break;
            case 2:  
                tmpValue = (params.dataSource==BTS_BOOT_DATA_SOURCE_BTS)? 0:1;
                i = promptParamNum ("dataSource(0-bts/1ems)", &tmpValue, FALSE); 
                if ( tmpValue != 0 && tmpValue!=1 )
                {
                    printf(" invalude input\n");
                    i = 0;
                }
                else
                {
                    params.dataSource = (tmpValue==0)? BTS_BOOT_DATA_SOURCE_BTS: BTS_BOOT_DATA_SOURCE_EMS;
                }
                printf("\n");
                break;

            case 3: 
                i = promptParamString ("main EMS IP", emsip, sizeof (emsip));
                if ( NULL == strchr(emsip, '.') || ERROR == checkInetAddrField(emsip, FALSE) )
                {
                    printf ("Error: invalid inet address\n");
                    i=0;
                }
                else
                {
                    params.emsIp = inet_addr(emsip);
                }
                printf("\n");
                break;

            case 4:  
                i = promptParamNum ("main EMS RcvPort(D=3999)", &(params.emsRcvPort), FALSE); 
                if ( params.emsRcvPort < 1000 || params.emsRcvPort > 20000 )
                {
                    printf("ems port number valid range 1000 ~ 20000\n");
                    i=0;
                }
                printf("\n");
                break;

            case 5: 
                i = promptParamString ("backup EMS IP", bakemsip, sizeof (bakemsip));
                if ( bakemsip[0] == EOS )
                {
                    /*不配置备用ems ip和port*/
                    params.bakemsIP      = 0;
                    i=2;
                    break;
                }
                if ( NULL == strchr(bakemsip, '.') || ERROR == checkInetAddrField(bakemsip, FALSE) )
                {
                    printf ("Error: invalid inet address\n");
                    i=0;
                }
                else if(inet_addr(bakemsip)==params.emsIp)
                {
                     printf("Error: backup EMS IP can't be equal main EMS IP\n");
                     i=0;
                }
                else
                {
                    params.bakemsIP = inet_addr(bakemsip);
                }
                printf("\n");
                break;

            case 6:  
                i = promptParamNum ("backup EMS RcvPort(D=3999)", &(params.bakemsRcvPort), FALSE); 
                if ( params.bakemsRcvPort < 1000 || params.bakemsRcvPort > 20000 )
                {
                    printf("ems port number valid range 1000 ~ 20000\n");
                    i=0;
                }
                printf("\n");
                break;

            case 7: 
                i = promptParamNum ("btsRcvPort(D=8002)", &(params.btsRcvPort), FALSE);
                if ( params.btsRcvPort < 1000 || params.btsRcvPort>20000 )
                {
                    printf("bts port number valid range 1000 ~ 20000\n");
                    i=0;
                }
                printf("\n");
                break;      

            case 8:  
                tmpValue = dateTime.year;
                i = promptParamNum ("rtc year", &tmpValue, FALSE); 
                dateTime.year = tmpValue;
                if ( dateTime.year < 2006 )
                {
                    printf("invalid input\n");
                    i=0;
                }
                printf("\n");
                break;
            case 9: 
                tmpValue = dateTime.month;
                i = promptParamNum ("rtc month", &tmpValue, FALSE);
                dateTime.month = tmpValue;
                if ( dateTime.month > 12 || dateTime.month <= 0 )
                {
                    printf("invalid input\n");
                    i=0;
                }
                printf("\n");
                break;
            case 10:  
                tmpValue = dateTime.day;
                i = promptParamNum ("rtc day", &tmpValue, FALSE); 
                dateTime.day = tmpValue;

                if ( (1 == dateTime.month || 3 == dateTime.month || 5 == dateTime.month || 7 == dateTime.month ||
                8 == dateTime.month || 10 == dateTime.month || 12 == dateTime.month) && (dateTime.day > 31) )
                {
                    printf("Day out of range (1-31)\n");
                    i=0;
                }
                if ( (4 == dateTime.month || 6 == dateTime.month || 9 == dateTime.month || 11 == dateTime.month)
                && (dateTime.day > 30) )
                {
                    printf("Day out of range (1-30)\n");
                    i=0;
                }
                if ( (2 == dateTime.month) && (0 != dateTime.year % 4) && (dateTime.day > 28) )
                {
                    printf("Day out of range (1-28)\n");
                    i=0;
                }
                if ( (2 == dateTime.month) && (0 == dateTime.year % 4) && (dateTime.day > 29) )
                {
                    printf("Day out of range (1-29)\n");
                    i=0;
                }
                printf("\n");
                break;

            case 11:  
                tmpValue = dateTime.hour;
                i = promptParamNum ("rtc hour", &tmpValue, FALSE); 
                dateTime.hour = tmpValue;
                if ( dateTime.hour > 23 )
                {
                    printf(" Hour out of range\n");
                    i=0;
                }
                printf("\n");
                break;
            case 12:  
                tmpValue = dateTime.minute;
                i = promptParamNum ("rtc minute", &tmpValue, FALSE); 
                dateTime.minute = tmpValue;
                if ( dateTime.minute > 59 )
                {
                    printf(" Minute out of range\n");
                    i=0;
                }
                printf("\n");
                break;
            case 13: 
                tmpValue = dateTime.second;
                i = promptParamNum ("rtc second", &tmpValue, FALSE); 
                dateTime.second = tmpValue;
                if ( dateTime.second > 59 )
                {
                    printf(" Second out of range\n");
                    i=0;
                }
                printf("\n");
                break;
            case 14:
                i = promptParamNum ("Reboot BTS when WAN IF disconnected (0-No, 1-Yes)", &(params.rebootBtsWanIfDisc), FALSE);
                if ( params.rebootBtsWanIfDisc > 1 )
                {
                    printf("Only 0 or 1 if valid\n");
                    i=0;
                }
                printf("\n");
                break;      

            case 15:
                i = promptParamNum ("permit PPPoE (0-No, 1-Yes)", &(params.permitPPPoE), FALSE);
                if ( params.permitPPPoE > 1 )
                {
                    printf("Only 0 or 1 if valid\n");
                    i=0;
                }
                printf("\n");
                break;      

            case 16:
                i = promptParamNum ("filter STP packet (0-No, 1-Yes)", &(params.filterSTP), FALSE);
                if ( params.filterSTP > 1 )
                {
                    printf("Only 0 or 1 if valid\n");
                    i=0;
                }
                printf("\n");
                break;      
            case 17:
                i = promptParamNum ("Reboot BTS when FTP is down (0-No, 1-Yes)", &(params1.rebootBtsIfFtpDown), FALSE);
                if ( params1.rebootBtsIfFtpDown> 1 )
                {
                    printf("Only 0 or 1 if valid\n");
                    i=0;
                }
                printf("\n");
                break;    
            case 18:               
                i = promptParamNum ("Permit Use when Sag is down (0-No, 1-Yes)", &(params1.permitUseWhenSagDown), FALSE);
                if ( params1.rebootBtsIfFtpDown> 1 )
                {
                    printf("Only 0 or 1 if valid\n");
                    i=0;
                }
                printf("\n");
                break;  
	       case 19:  
                i = promptParamNum ("RRU Antenna Num (4---4channels,8---8 channels)", &(params1.RRU_Antenna), FALSE); 
                if (( params1.RRU_Antenna !=4 )&&( params1.RRU_Antenna !=8 ))
                {
                    printf("Channel Err  must 4 or 8 \n");
                    i = 0;
                }
                printf("\n");
                break;
           case 20:  //zengjihan 20120801 for GPSSYNC
                i = promptParamNum ("slaveBTSID: If input hex number, please add 0x!!!The system defaults decimal number!", &(params1.slaveBTSID), TRUE); 
                params1.slaveBTSIDFLAG = NVRAM_VALID_PATTERN;
                printf("\n");
                break;
            default: i = -99; break;
        }

        /* check for QUIT */

        if ( i == -99 )
        {
            printf ("\n");
            break;
        }

        /* move to new field */

        if ( i != -98 ) n += i;
        if ( i<0 )
        {
            i=0;
        }

    }/**forever******/

    /* update bts config data */
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
    /*生成crc，写入 jy080918*/
    cfGetCrc((char*)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));   
    
    if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open when write\"%s\".\n", FileName);                
    }
    else
    {
        fwrite( (const void * )NVRAM_BASE_ADDR_APP_PARAMS, 1, (sizeof(params)+6), fd); 
        fflush(fd);
        fclose (fd);
    }
 
     /* update bts config para */
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_PARA_PARAMS, (char *)&params1, sizeof(params1));
    /*生成crc，写入 jy080918*/
    cfGetCrc((char*)NVRAM_BASE_ADDR_PARA_PARAMS, sizeof(params1));    
    if ((fd = fopen (FileName1, "w+")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open when write\"%s\".\n", FileName1);                
    }
    else
    {
        fwrite( (const void * )NVRAM_BASE_ADDR_PARA_PARAMS, 1, (sizeof(params1)+6), fd); 
        fflush(fd);
        fclose (fd);
    }
    /* update RTC time */
      if((Hard_VerSion<5)||(Hard_VerSion>15))
      	{
    rtcDateSet(dateTime.year, dateTime.month, dateTime.day, 0/*dayOfWeek*/);
    rtcTimeSet(dateTime.hour, dateTime.minute, dateTime.second);
      	}
      else
      	{
      	           
                    pbuffer[0] = dateTime.second;//&0x7f;  /******s********/
                    pbuffer[1] = dateTime.minute;//&0x7f;  /******min*******/
                    pbuffer[2] = dateTime.hour;//&0x3f;  /*********hour*******/
                    pbuffer[3] = dateTime.day;//&0x3f; /*******day********/
                    pbuffer[4] = 5;//&0x7; /****week day*****/
                    pbuffer[5] = dateTime.month;//&0x1f; /*******month*******/
                    pbuffer[6] = (dateTime.year-2000);//&0xff; /**year***/
                    bspSetTime(pbuffer);
      	}

                    time_s.tm_sec = dateTime.second;
    			time_s.tm_min = dateTime.minute;
   			 time_s.tm_hour = dateTime.hour;

   			 time_s.tm_mday = dateTime.day;
   			 time_s.tm_mon = dateTime.month - 1;

   			 time_s.tm_year = dateTime.year - 1900;
     
 
    			time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    			secCount = mktime(&time_s);
    			    timeSpec.tv_sec = secCount;    
   			 timeSpec.tv_nsec = 0;
    			clock_settime(CLOCK_REALTIME, &timeSpec);
/*}*/

    isConfigChanged = TRUE;

}  


void sysBootConfigModify()
{
    int i;
    int n=0;
    T_BootLoadState bootState;
    int tmpValue;

    bspNvRamRead((char *)(&bootState), (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    if ( bootState.nvramSafe != NVRAM_VALID_PATTERN )
    {
        memset(&bootState, 0, sizeof(bootState));
        bootState.nvramSafe = NVRAM_VALID_PATTERN;
    }

    printf ("\n'.' = clear field;  '-' = go to previous field;  ^D = quit\n\n");

    /* prompt the user for each item;
     *   i:  0 = same field, 1 = next field, -1 = previous field,
     *     -98 = error, -99 = quit
     */

    FOREVER
    {
        switch ( n )
        {
            case 0:
                tmpValue = bootState.bootPlane;
                i = promptParamNum ("bootPlane(0-A, 1-B)", &tmpValue, FALSE); 
                if ( 0 != tmpValue && 1 != tmpValue )
                {
                    printf("Invalid plane selection\n");
                    i=0;
                }
                else
                {
                    bootState.bootPlane = (tmpValue==0)? BOOT_PLANE_A: BOOT_PLANE_B;
                }
                break;
            case 1: 
                i = promptParamNum ("resetNum", &(bootState.resetNum), FALSE); 
                break;

            case 2:
                tmpValue = (bootState.workFlag==LOAD_STATUS_TEST_RUN)?0:1;
                i = promptParamNum ("workFlag(0-test/1-verified)", &tmpValue, FALSE);
                if ( 0!= tmpValue && 1!= tmpValue )
                {
                    printf("invalid workFlag\n");
                    i=0;
                }
                else
                {
                    bootState.workFlag = (tmpValue==0)? LOAD_STATUS_TEST_RUN: LOAD_STATUS_VERIFIED;
                }
                break;

            case 3:
                printf("Plane A Version:     %d\n", bootState.LoadVersion_A);
                printf("Plane B Version:     %d\n", bootState.LoadVersion_B);
                i=1;
                break;
       #if 0
            case 4://wangwenhua add 2010 0210
            	    tmpValue = bootState.boot_source;
                i = promptParamNum ("bootPlane(0-ftp, 1-CF)", &tmpValue, FALSE); 
                if ( 0 != tmpValue && 1 != tmpValue )
                {
                    printf("Invalid plane selection\n");
                    i=0;
                }
                else
                {
                    bootState.boot_source = (tmpValue==0)? FTP_BOOT: CF_BOOT;
                }
#endif
            default: 
                i = -99; break;
        }

        /* check for QUIT */

        if ( i == -99 )
        {
            printf ("\n");
            break;
        }

        /* move to new field */

        if ( i != -98 ) n += i;
        if ( i<0 )
        {
            i=0;
        }

    }/**forever******/

    isConfigChanged = TRUE;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
}

char *BtsResetStr[] = {"HW WATCHDOG",
                    "SW WATCHDOG",
                    "POWER ON   ",
                    "SW NORMAL  ",
                    "SW ALARM   ",
                    "SW ABNORMAL",
                    "BOOTUP FAIL",
                    "EMS REQUEST",
                    "UNKNOWN    "};

void sysBootStateShow()
{
	T_BootLoadState bootState;
    int index;

	bspNvRamRead((char *)(&bootState), (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));

	printf("Safe Pattern: %x\n", bootState.nvramSafe);
	printf("Boot Plane  : %d ( 0 - BTSA, 1 - BTSB) \n", bootState.bootPlane);
	printf("WorkFlag    : %d ( 0 - test run, 1 - verified) \n", bootState.workFlag);
	printf("ResetNum    : %d\n", bootState.resetNum);
    index = bootState.btsRstReason - RESET_REASON_HW_WDT;
    if ( (unsigned int)(bootState.btsRstReason - RESET_REASON_HW_WDT) > (RESET_REASON_EMS-RESET_REASON_HW_WDT))
    {
        index = 8;
    }
	printf("ResetReason : %s\n", BtsResetStr[index]);
	printf("ResetFlag   : %d\n", bootState.resetFlag);
	printf("Version_A   : %08x\n", bootState.LoadVersion_A);
	printf("Version_B   : %08x\n", bootState.LoadVersion_B);
	#if 0
	if(bootState.boot_source == 0)
	{
		   printf("Boot From FTP\n");
	}
	else
	{
		    printf("Boot From CF\n");
	}
	#endif
}

/**Get*****/
int bspGetBtsUDPRcvPort()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->btsRcvPort;
}
int bspGetMainEmsUDPRcvPort()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->emsRcvPort;
}

int bspGetBakEmsUDPRcvPort()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->bakemsRcvPort;
}

int bspGetBtsPubIp()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->btsIp;
}

int bspGetBtsPubPort()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->btsPort;
}



int bspGetNvramSafe()
{
    return((T_BootLoadState *)NVRAM_BASE_ADDR_BOOT_STATE)->nvramSafe;
}
int bspGetResetNum()
{
    return((T_BootLoadState *)NVRAM_BASE_ADDR_BOOT_STATE)->resetNum;
}
BOOT_PLANE bspGetBootPlane()
{
    T_BootLoadState* bootParam = (T_BootLoadState*)NVRAM_BASE_ADDR_BOOT_STATE;

    if (bootParam->bootPlane == BOOT_PLANE_B)
    {
        return BOOT_PLANE_B;
    }
    else
    {
        return BOOT_PLANE_A;
    }
}



unsigned int    bspGetBootSource()
{
      T_BootLoadState* bootParam = (T_BootLoadState*)NVRAM_BASE_ADDR_BOOT_STATE;

    if (bootParam->boot_source == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
LOAD_STATUS bspGetWorkFlag()
{
    return((T_BootLoadState *)NVRAM_BASE_ADDR_BOOT_STATE)->workFlag;
}

int bspGetResetFlag()
{
    return((T_BootLoadState *)NVRAM_BASE_ADDR_BOOT_STATE)->resetFlag;
}

int bspGetMainEmsIpAddr()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->emsIp;
}
int bspGetBakEmsIpAddr()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->bakemsIP;
}
int bspGetBtsID()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->btsId;
}
BTS_BOOT_DATA_SOURCE bspGetBootupSource()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->dataSource;
}
int bspGetVlanID()
{
    return((T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS)->vlanId;
}
RESET_REASON bspGetBtsResetReason()
{
    return((T_BootLoadState *)NVRAM_BASE_ADDR_BOOT_STATE)->btsRstReason;
}


int bspGetAtaState()
{
/***return -1 error, 1 ready***/    
//    int state;
#if 0
    state = *(int *)(CPLD_BASE_ADRS + CPLD_ATA_STATE_REG)/********/;
    if ( (state & (1<<5)) != 0 )
    {   /**bit5,low is ready**/
        return -1;  
    }
#endif
    return 1;
}



/**set*****/

/* update bts config data */

void bspSetBtsUDPRcvPort(int portNum)
{
    if ( portNum >= 1000 && portNum < 20000 )
    {
        T_NVRAM_BTS_CONFIG_DATA params;
        char FileName[20] = "/ata0a/nvEmsData";    
        FILE *fd;
        
        bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
        params.btsRcvPort = portNum;

        bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
        /*生成crc，写入 jy080918*/          
        cfGetCrc((char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));        
        if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
        {
            printErr ("\nCannot open when write\"%s\".\n", FileName);                
        }
        else
        {
            fwrite((const void *) NVRAM_BASE_ADDR_APP_PARAMS, 1,sizeof(params)+6, fd); 
            fflush(fd);
            fclose (fd);
        }
    }
    else
    {
        logMsg("BTS Receive Port Num out of range\n", 0,0,0,0,0,0);
    }
}


void bspSetBtsPubIp(int ipaddr)
{
    if ( ipaddr != 0x0000 && ipaddr != 0xFFFF )
    {
        T_NVRAM_BTS_CONFIG_DATA params;
        char FileName[20] = "/ata0a/nvEmsData";    
        FILE *fd;
        
        bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
        params.btsIp = ipaddr;

        bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
        /*生成crc，写入 jy080918*/        
        cfGetCrc((char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));        
        if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
        {
            printErr ("\nCannot open when write\"%s\".\n", FileName);                
        }
        else
        {
            fwrite((const void *) NVRAM_BASE_ADDR_APP_PARAMS, 1,sizeof(params)+6, fd); 
            fflush(fd);
            fclose (fd);
        }
    }
    else
    {
        logMsg("BTS Ip out of range\n", 0,0,0,0,0,0);
    }
}

void bspSetBtsPubPort(int port)
{
    if ( port >= 1000 && port < 20000 )
    {
        T_NVRAM_BTS_CONFIG_DATA params;
        char FileName[20] = "/ata0a/nvEmsData";    
        FILE *fd;
        
        bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
        params.btsPort= port;

        bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
        /*生成crc，写入 jy080918*/          
        cfGetCrc((char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));        
        if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
        {
            printErr ("\nCannot open when write\"%s\".\n", FileName);                
        }
        else
        {
            fwrite((const void *) NVRAM_BASE_ADDR_APP_PARAMS, 1,sizeof(params)+6, fd); 
            fflush(fd);
            fclose (fd);
        }
    }
    else
    {
        logMsg("BTS Receive Port Num out of range:%d\n", port,0,0,0,0,0);
    }
}

#if 0
void bspSetEmsUDPRcvPort(int portNum)
{
    if ( portNum >= 1000 && portNum < 20000 )
    {
        T_NVRAM_BTS_CONFIG_DATA params;
        bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
        params.emsRcvPort = portNum;

        bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
    }
    else
    {
        printf("BTS Receive Port Num out of range\n");
    }
}
#endif

void bspSetNvramSafe(int safeValue)
{
    T_BootLoadState bootState;

    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    bootState.nvramSafe = safeValue;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
}

void bspSetResetNum(int resetCount)
{
    T_BootLoadState bootState;

    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    bootState.resetNum = resetCount;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
}
void bspSetBootPlane(BOOT_PLANE bootPlane)
{
    T_BootLoadState bootState;

    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    bootState.bootPlane = bootPlane;
	bootState.LoadVersion_A = bspGetLoadVersion_A();
	bootState.LoadVersion_B = bspGetLoadVersion_B();
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));

}

void bspSetWorkFlag(LOAD_STATUS workState)
{
    T_BootLoadState bootState;

    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    bootState.workFlag = workState;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
}


void bspSetResetFlag(int temp)
{
    T_BootLoadState bootState;
    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    bootState.resetFlag = temp;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
}

void bspSetBtsID(int btsId)
{
    T_NVRAM_BTS_CONFIG_DATA params;
    char FileName[20] = "/ata0a/nvEmsData";    
    FILE *fd;
    
    bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
    params.btsId = btsId;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
    /*生成crc，写入 jy080918*/   
    cfGetCrc((char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));    
    if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open when write\"%s\".\n", FileName);                
    }
    else
    {
        fwrite( (const void *)NVRAM_BASE_ADDR_APP_PARAMS, 1,sizeof(params)+6, fd); 
        fflush(fd);
        fclose (fd);
    }
    
}

void bspSetBootupSource(BTS_BOOT_DATA_SOURCE temp)
{
    T_NVRAM_BTS_CONFIG_DATA params;
    char FileName[20] = "/ata0a/nvEmsData";    
    FILE *fd;
    
    bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
    params.dataSource = temp;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
    /*生成crc，写入 jy080918*/    
    cfGetCrc((char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));    
    if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open when write\"%s\".\n", FileName);                
    }
    else
    {
        rewind(fd);
        fwrite( (const void *)NVRAM_BASE_ADDR_APP_PARAMS, 1,sizeof(params)+6, fd); 
        fflush(fd);
        fclose (fd);
     //   printf("write ok:%x\n",temp);
    }
}

void bspSetVlanID(int vlanId)
{
    T_NVRAM_BTS_CONFIG_DATA params;
    char FileName[20] = "/ata0a/nvEmsData";    
    FILE *fd;
    
    bspNvRamRead((char*)&params, (char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));
    params.vlanId = vlanId;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_APP_PARAMS, (char *)&params, sizeof(params));
    /*生成crc，写入 jy080918*/      
    cfGetCrc((char *)NVRAM_BASE_ADDR_APP_PARAMS, sizeof(params));   
    if ((fd = fopen (FileName, "w+")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open when write\"%s\".\n", FileName);                
    }
    else
    {
        fwrite((const void *) NVRAM_BASE_ADDR_APP_PARAMS, 1,sizeof(params)+6, fd); 
        fflush(fd);
        fclose (fd);
    }
}

void bspSetBtsResetReason(RESET_REASON temp)
{
    T_BootLoadState bootState;
    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    if ( bootState.isResetReasonSet!=1)
    {    /* only save the first reset reason */
        bootState.btsRstReason = temp;
        bootState.isResetReasonSet = 1;
        bootState.resetNum++;
        
        bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
    }
}
/*********************wangwenhua add 20110429
将复位原因值是否设置清零



**************************************************/
void ClearResetReasonSet()
{
         T_BootLoadState bootState;
    	  bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));

        bootState.isResetReasonSet = 0;
      
        
        bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
  
}

void getIoOpt()
{
    int consoleInFd = ioGlobalStdGet(STD_IN);

    int oldOpts = ioctl(consoleInFd, FIOGETOPTIONS, 0);
    printf("current STD_In option is  %x\n", oldOpts);
}


void l3BootlineShow()
{
    bootParamsShow( (char*)BOOT_LINE_ADRS_L3);
}

void l2BootlineShow()
{
    bootParamsShow( (char*)BOOT_LINE_ADRS_L2);
}

#if 0
void check_i2c_eeprom()
{
    T_I2C_TABLE i2cData;
    unsigned short *buffer; 
    unsigned char  temp[4];
    unsigned short checksum;
    int i;

    i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (char *)(&i2cData), sizeof(i2cData));
    checksum = 0;
    buffer = (UINT16*)&i2cData;
    for ( i=1; i< sizeof(i2cData)/2; i++ )
    {
        checksum += buffer[i];
    }

    if ( checksum != i2cData.checkSum )
    {
        logMsg("\n\ni2c eeprom checksum error, read: 0x%x, compute: 0x%x !!!!!!\n\n",  i2cData.checkSum, checksum, 0,0,0,0);
    }
}


STATUS bspGetVersionNum(UINT16* version)
{
    T_I2C_TABLE i2cData;
    return i2c_read(I2C_E2PROM_DEV_ADDR, 
                    (UINT32)(&i2cData.verNum) - (UINT32)(&i2cData),
                    (UINT8*)version,
                    sizeof(i2cData.verNum)
                    );
}

STATUS bspGetPcbVersion(char *buf)
{
    T_I2C_TABLE i2cData;
    return i2c_read(I2C_E2PROM_DEV_ADDR, 
                    (UINT32)(&i2cData.PCBVersion) - (UINT32)(&i2cData),
                    buf,
                    sizeof(i2cData.PCBVersion)
                    );
}
STATUS bspGetDeviceID(unsigned char *device)
{
    T_I2C_TABLE i2cData;
    FAST int i;
    unsigned short checksum;
    unsigned short *buffer; 
    unsigned char tail;
    unsigned char deviceID[20];

    if(0 != i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
		deviceID[19] = 0xff;

    ioctl (STD_IN, FIOFLUSH, 0 ); 

    buffer = (unsigned short *)&i2cData;
    checksum  = 0;
    for ( i=1; i<sizeof(i2cData)/2; i++ )
    {   
        checksum += buffer[i];
    }
    for(i = 0;i<19;i++)
    {
    	deviceID[i] = i2cData.DeviceID[i];
    }
	if(checksum != i2cData.checkSum) 
	{
		deviceID[19] = 0xff;
		printf("\nchecksum error \n%x should be %d",checksum,i2cData.checkSum);
	}
	else if(0x00 == i2cData.DeviceID[0])
		deviceID[19] = 0xfe;
	else deviceID[19] = 0x00;
#if 0
for(i=0;i<20;i++)
	printf("\ndeviceID[%d] = %x",i,i2cData.DeviceID[i] );
#endif
if(0 != GenerateCRC8for8N(i2cData.DeviceID,20))
	{
		deviceID[19] = 0xff;
		printf("\ncrc error !");
	}

    strcpy(device,deviceID);
    #if 0
	printf("\ndeviceID = %s",device);
for(i=0;i<20;i++)
	printf("\ndevice[%d] = %x",i,device[i] );

printf("\ndevice[19]= %x ",device[19]);
#endif
}



STATUS bspGetSerialNum(UINT16 *serial)
{
    T_I2C_TABLE i2cData;
    return i2c_read(I2C_E2PROM_DEV_ADDR, 
                    (UINT32)(&i2cData.serialNum) - (UINT32)(&i2cData),
                    (UINT8*)serial,
                    sizeof(i2cData.serialNum)
                    );
}

STATUS bspGetProductionDate(UINT8* month, UINT8* day, UINT16* year)
{
    T_I2C_TABLE i2cData;
    if (ERROR == i2c_read(I2C_E2PROM_DEV_ADDR, 
                    (UINT32)(&i2cData.prodDate_Month) - (UINT32)(&i2cData),
                    month,
                    sizeof(i2cData.prodDate_Month)
                    )
        )
    {
        return ERROR;
    }
    if (ERROR == i2c_read(I2C_E2PROM_DEV_ADDR, 
                    (UINT32)(&i2cData.prodDate_Day) - (UINT32)(&i2cData),
                    day,
                    sizeof(i2cData.prodDate_Day)
                    )
        )
    {
        return ERROR;
    }
    if (ERROR == i2c_read(I2C_E2PROM_DEV_ADDR, 
                    (UINT32)(&i2cData.prodDate_Year) - (UINT32)(&i2cData),
                    (UINT8*)year,
                    sizeof(i2cData.prodDate_Year)
                    )
        )
    {
        return ERROR;
    }
    return OK;
}
#endif
extern STATUS bootScanNum(FAST char **ppString,int *pValue,FAST BOOL hex);

STATUS promptEnet( unsigned char unit ,UINT8 *addr)
{
//    int ix;
    char line [160];
    char *pLine;
    int value0,value1, value2;
   if(unit==0)
   	{
    addr[0] = 0x00/*MCWILL_ENET0*/;
    addr[1] = 0x18/*MCWILL_ENET1*/;
    addr[2] = 0x12/*MCWILL_ENET2*/;
   // addr[3] = 0xfb;
   	}
   else if(unit==1)
   	{
   	       addr[0] = 0x00/*MCWILL_ENET0*/;
    		addr[1] = 0xa0/*MCWILL_ENET1*/;
    		addr[2] = 0x1e/*MCWILL_ENET2*/;
    		addr[3] = 0x01;
    		
   	}

    printf ("  (hex) %02x,%02x,%02x,", addr[0], addr[1], addr[2]);

    /* start on fourth byte of enet addr */
    fioRdString (STD_IN, line, MAX_LINE);
    pLine = line + strlen (line) - 1;       /* point at last char */
    while ( (pLine >= line) && (*pLine == ' ') )
    {
        *pLine = EOS;
        pLine--;
    }

    pLine = line;
    skipSpace (&pLine);

    if (   (getArg (&pLine, &value0, HEX, FALSE) == OK) 
         &&(getArg (&pLine, &value1, HEX, FALSE) == OK) 
         &&(getArg (&pLine, &value2, HEX, FALSE) == OK) 
       )
    {
        addr[3] = value0;
        addr[4] = value1;
        addr[5] = value2;
        return OK;
    }
    return ERROR;
}

extern int i2cWriteByte(unsigned char *ptr,unsigned int len);

extern int i2cReadByte(unsigned char *ptr,unsigned int len);
int WriteI2cData(T_I2C_TABLE *i2cDataPtr)
{

    UINT8* bufPtr;
 //   UINT8 pageNum;
  //  int i, offset;
   int result =0;
    bufPtr = (UINT8*)i2cDataPtr;
#if 0
    pageNum = sizeof(T_I2C_TABLE)/I2C_WRITE_PAGE_SIZE;
    for (i=0, offset=0; i<pageNum; i++)
    {
        i2c_write(I2C_E2PROM_DEV_ADDR, 
                  i*I2C_WRITE_PAGE_SIZE,
                  &bufPtr[offset],
                  I2C_WRITE_PAGE_SIZE
                 );
        offset +=  I2C_WRITE_PAGE_SIZE;
        taskDelay(50);
    }

    i2c_write(I2C_E2PROM_DEV_ADDR, offset, &bufPtr[offset], sizeof(T_I2C_TABLE)-offset);
#endif
  result = i2cWriteByte(bufPtr,sizeof(T_I2C_TABLE));
  printf("result:%d\n",result);
return result;
}

unsigned char teste2prom( )
{
      	T_I2C_TABLE i2cData;
  //  FAST int n = 0;
//    FAST int i;
 //   unsigned short checksum;
 //   unsigned short *buffer; 
  //  unsigned int tempValue;
  //  unsigned int pageNum;
 //     T_EMAC Mac;
 //   int tmpValue;
   memset((UINT8 *)&i2cData,0xff,sizeof(T_I2C_TABLE));
   i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
   WriteI2cData(&i2cData);
   return 1;
}
STATUS GetBtsMac(unsigned char *pMac)
{
    T_I2C_TABLE i2cData;
   /* char DevID[21];*/
     if(pMac==NULL)
     	{
     	    printf("pMac is NULL\n");
           return ERROR;
     	}
#if 0
    if ( 0 != i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        printf("I2C read failed\n");
        return ERROR;
    }
    #endif

    if ( 0 != i2cReadByte( (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        printf("I2C read failed\n");
        return ERROR;
    }
    #if 0
      printf("L3 MAC  :     %02x-%02x-%02x-%02x-%02x-%02x\n",
             i2cData.L3_mac[0], i2cData.L3_mac[1], i2cData.L3_mac[2],
             i2cData.L3_mac[3], i2cData.L3_mac[4], i2cData.L3_mac[5]);
      #endif
     memcpy(pMac,&i2cData.L3_mac[0],6);
	 return OK;
	
}
/*************************************************
该函数主要实现如下功能
1、将MAC地址保存在e2prom中
2、同时将MAC地址保存在NVRAM中，实现了以前的setMac功能


*******************************************************/
void sysI2cConfig()
{
#if 1
	T_I2C_TABLE i2cData;
    FAST int n = 0;
    FAST int i;
 //   unsigned short checksum;
    unsigned short *buffer; 
    unsigned int tempValue;
  //  unsigned int pageNum;
      T_EMAC Mac;
  //  int tmpValue;
   i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
 //   i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (UINT8 *)&i2cData, sizeof(T_I2C_TABLE));

    printf ("\n'.' = clear field;  '-' = go to previous field;  ^D = quit\n\n");

    /* prompt the user for each item;
     *   i:  0 = same field, 1 = next field, -1 = previous field,
     *     -98 = error, -99 = quit
     */
 

    bspNvRamRead((char *)(&Mac), (char *)BOOT_LINE_ADRS_L2, sizeof(Mac));
    if ( Mac.nvramSafe != NVRAM_VALID_PATTERN )
    {
        memset(&Mac, 0, sizeof(Mac));
        Mac.nvramSafe = NVRAM_VALID_PATTERN;
    }

    ioctl (STD_IN, FIOFLUSH, 0 ); 

    FOREVER
    {
        switch ( n )
        {
            case 0:  
                tempValue = i2cData.verNum;
                i = promptParamNum(" Table Verion", 
                                   &tempValue, 
                                   FALSE); 
                if (tempValue > 0x65535)
                {
                    printf("Invalid input\n");
                    i=0;
                }
                else
                {
                    i2cData.verNum = tempValue;
                }
                break;
            case 1:  
                i = promptParamString ("PCB Version", 
                                       i2cData.PCBVersion, 
                                       sizeof (i2cData.PCBVersion));   
                break;
            case 2:
                tempValue = i2cData.serialNum;
                i = promptParamNum ("serialNum", 
                                    &tempValue, 
                                    FALSE);
                if (tempValue > 0xFFFF)
                {
                    printf("valid range 0~65535\n");
                    i=0;
                }
                else
                {
                    i2cData.serialNum = tempValue;
                }
                break;
            case 3:
                tempValue = i2cData.prodDate_Year;
                i = promptParamNum ("production Year", 
                                    &tempValue,
                                    FALSE); 
                if ( tempValue < 2006 || tempValue> 0xffff )
                {
                    printf("invalid input\n");
                    i=0;
                }
                else
                {
                    i2cData.prodDate_Year = tempValue;
                }
                break;
            case 4:
                tempValue = i2cData.prodDate_Month;
                i = promptParamNum ("production Month",
                                    &tempValue,
                                    FALSE);
                if ( tempValue > 12 || tempValue <= 0 )
                {
                    printf("invalid input\n");
                    i=0;
                }
                else
                {
                    i2cData.prodDate_Month = tempValue;
                }
                break;

            case 5:
                tempValue = i2cData.prodDate_Day;
                i = promptParamNum ("production Day",
                                    &tempValue,
                                    FALSE);
                if ( (1 == i2cData.prodDate_Month || 3 == i2cData.prodDate_Month 
                      || 5 == i2cData.prodDate_Month || 7 == i2cData.prodDate_Month
                      ||8 == i2cData.prodDate_Month || 10 == i2cData.prodDate_Month
                      || 12 == i2cData.prodDate_Month) && tempValue > 31 )
                {
                    printf("Day out of range (1-31)\n");
                    i=0;
                }
                if ( (4 == i2cData.prodDate_Month || 6 == i2cData.prodDate_Month
                     || 9 == i2cData.prodDate_Month || 11 == i2cData.prodDate_Month)
                     && (tempValue > 30) )
                {
                    printf("Day out of range (1-30)\n");
                    i=0;
                }
                if ( (2 == i2cData.prodDate_Month) && (0 != i2cData.prodDate_Year % 4) 
                     && (tempValue > 28) )
                {
                    printf("Day out of range (1-28)\n");
                    i=0;
                }
                if ( (2 == i2cData.prodDate_Month) && (0 == i2cData.prodDate_Year % 4) 
                     && (tempValue> 29) )
                {
                    printf("Day out of range (1-29)\n");
                    i=0;
                }
                if ( 0 != i )
                {
                    i2cData.prodDate_Day = tempValue;
                }
                break;

            case 6:
                printf ("L3 MAC              : %02x-%02x-%02x-%02x-%02x-%02x  ",
                         i2cData.L3_mac[0], i2cData.L3_mac[1], i2cData.L3_mac[2],
                         i2cData.L3_mac[3], i2cData.L3_mac[4], i2cData.L3_mac[5]);
                promptEnet(0,i2cData.L3_mac);
                memcpy(Mac.L3_mac,i2cData.L3_mac,6);
                break;

            case 7:  
                printf ("L2 MAC              : %02x-%02x-%02x-%02x-%02x-%02x  ",
                         i2cData.L2_mac[0], i2cData.L2_mac[1], i2cData.L2_mac[2],
                         i2cData.L2_mac[3], i2cData.L2_mac[4], i2cData.L2_mac[5]);
                promptEnet(1,i2cData.L2_mac);
                memcpy(Mac.L2_mac,i2cData.L2_mac,6);
                break;
       #if 0
           case 8:
           	      promptParamString ("DeviceID", 
                                       i2cData.DeviceID, 
                                       sizeof (i2cData.DeviceID));   
           	        i2cData.DeviceID[19] = GenerateCRC8for8N(i2cData.DeviceID,19);
       #endif
            default: i = -99; break;
        }

        /* check for QUIT */

        if ( i == -99 )
        {
            printf ("\n");
            break;
        }

        /* move to new field */

        if ( i != -98 ) n += i;

    }/**forever******/

    memset(&i2cData.reserved0, 0, sizeof(i2cData.reserved0));
    memset(&i2cData.reserved1, 0, sizeof(i2cData.reserved1));

    buffer = (unsigned short *)&i2cData;
    i2cData.checkSum  = 0;
    for ( i=1; i<sizeof(i2cData)/2; i++ )
    {   
        i2cData.checkSum += buffer[i];
    }
    bspNvRamWrite((char *)BOOT_LINE_ADRS_L2, (char *)&Mac, sizeof(Mac));
    WriteI2cData(&i2cData);
#endif
}  

void sysDevConfig()
{
#if 1
    T_I2C_TABLE i2cData;
  //  FAST int n = 0;
    FAST int i;
  //  unsigned short checksum;
    unsigned short *buffer; 
  //  unsigned int tempValue;
   // unsigned int pageNum;
    i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
  //  i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (UINT8 *)&i2cData, sizeof(T_I2C_TABLE));

    ioctl (STD_IN, FIOFLUSH, 0 ); 

    promptParamString ("DeviceID", 
                                       i2cData.DeviceID, 
                                       sizeof (i2cData.DeviceID));   



    buffer = (unsigned short *)&i2cData;
    i2cData.checkSum  = 0;
    for ( i=1; i<sizeof(i2cData)/2; i++ )
    {   
        i2cData.checkSum += buffer[i];
    }
   
    i2cData.DeviceID[19] = GenerateCRC8for8N(i2cData.DeviceID,19);
   //    return;
    WriteI2cData(&i2cData);
#endif
}  
STATUS sysI2cClear()
{
#if 1
    T_I2C_TABLE i2cData;
    memset((void *)&i2cData, 0, sizeof(i2cData));

    WriteI2cData(&i2cData);
#endif
    return OK;
}


STATUS sysI2cShow()
{
#if 1
    T_I2C_TABLE i2cData;
//   int i =0;
    unsigned char DevID[21];
  //     i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
    if ( 0 != i2cReadByte( (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        printf("I2C read failed\n");
        return ERROR;
    }
 
    printf("\n");
    printf("Checksum:     0x%x\n", i2cData.checkSum);
    printf("VerNum  :     %x\n", i2cData.verNum);
    printf("PCB version:  %s\n", i2cData.PCBVersion);
    printf("Serial #:     %d\n", i2cData.serialNum);
    printf("ProdData:     %d/%d/%d\n", i2cData.prodDate_Year, 
                              i2cData.prodDate_Month, i2cData.prodDate_Day);
    printf("L3 MAC  :     %02x-%02x-%02x-%02x-%02x-%02x\n",
             i2cData.L3_mac[0], i2cData.L3_mac[1], i2cData.L3_mac[2],
             i2cData.L3_mac[3], i2cData.L3_mac[4], i2cData.L3_mac[5]);

    printf("L2 MAC  :     %02x-%02x-%02x-%02x-%02x-%02x\n",
             i2cData.L2_mac[0], i2cData.L2_mac[1], i2cData.L2_mac[2],
             i2cData.L2_mac[3], i2cData.L2_mac[4], i2cData.L2_mac[5]);
  
    memcpy(DevID ,  i2cData.DeviceID,21);
    DevID[19] = 0x00;
   printf("DeviceID: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", DevID[0],DevID[1],DevID[2],DevID[3],DevID[4],DevID[5],DevID[6],
    	DevID[7],DevID[8],DevID[9],DevID[10],DevID[11],DevID[12],DevID[13],DevID[14],DevID[15],DevID[16],DevID[17],DevID[18],DevID[19]);
#endif
}
/********************
得到硬件版本的最后一位，做为版本号

如PCB version:  BPBH01.01.05   则取5作为返回值

******************/
unsigned char getHardWareVersion()
{
    T_I2C_TABLE i2cData;
 //  int i =0;
   unsigned char hardVer = 0;
  //     i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
    if ( 0 != i2cReadByte( (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        printf("I2C read failed\n");
        return 0;
    }
    #if 0
    for(i = 0; i< 14; i++)
    	{
    			hardVer = i2cData.PCBVersion[i];
    			  printf("hardVer:,%d %02x\n",i,hardVer);
    	}
    #endif
     hardVer = (i2cData.PCBVersion[11] - 0x30);
    printf("hardVer Last Num:%02x\n",hardVer);  
        return hardVer;
    
}
void  bspGetBPBSerial( char *ptr)
{
       T_I2C_TABLE i2cData;
 //  int i =0;
  //  unsigned char DevID[21];
  //     i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
    if ( 0 != i2cReadByte( (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        printf("I2C read failed\n");
    //    return 0;
     //   return ERROR;
    }
    memcpy(ptr, i2cData.PCBVersion,14);
   // return 1;
}

unsigned char  bspGetDeviceID(unsigned char *ptr)
{
     T_I2C_TABLE i2cData;
 //  int i =0;
  //  unsigned char DevID[21];
  //     i2cReadByte((UINT8 *)&i2cData, sizeof(T_I2C_TABLE));
    if ( 0 != i2cReadByte( (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        printf("I2C read failed\n");
      //  return ERROR;
      return 0;
    }
    memcpy(ptr, i2cData.DeviceID,20);
    return 1;
}
int bspGetLoadVersion_A()
{
    FILE *pFVersion = fopen("/ata0a/btsA/loadVersion.txt" , "r");
    char strVersion[16];
    memset(strVersion, 0, 16);
    if(NULL != pFVersion)
    {
        fread(strVersion, 16, 1, pFVersion);
        return inet_addr(strVersion);
    }
    else
    {
		return 0;
    }
}



int bspGetLoadVersion_B()
{
    FILE *pFVersion = fopen("/ata0a/btsB/loadVersion.txt" , "r");
    char strVersion[16];
    memset(strVersion, 0, 16);
    if(NULL != pFVersion)
    {
        fread(strVersion, 16, 1, pFVersion);
        return inet_addr(strVersion);
    }
    else
    {
		return 0;
    }
}

STATUS sysSetL3Bootline()
{
    int length;
    int oriLen = strlen( (char *)BOOT_LINE_ADRS_L3);
    char l3FileName[20] = "/ata0a/l3Bootline";    
    FILE *fdl3;
    setboolineFlag();
    memcpy((char *)BOOT_LINE_ADRS, (char *)BOOT_LINE_ADRS_L3, oriLen);
    bootParamsPrompt ( (char *)BOOT_LINE_ADRS);
    length = strlen( (char *)BOOT_LINE_ADRS);
    if ( length <= BOOT_LINE_SIZE)
    {
       /* update bts config data */
        bspNvRamWrite((char *)BOOT_LINE_ADRS_L3, (char *)BOOT_LINE_ADRS, length+1);
        /*生成crc，写入l3bootline   jy080918*/
       cfGetCrc((char *)BOOT_LINE_ADRS_L3, 0);
       if ((fdl3 = fopen (l3FileName, "w+")) == (FILE *)ERROR)
       {
           printErr ("\nCannot open when write\"%s\".\n", l3FileName);                
       }
       else
        {
           fwrite( (const void *)BOOT_LINE_ADRS_L3, 1,BOOT_LINE_SIZE, fdl3); 
           fflush(fdl3);
           fclose (fdl3);
        }        
        printf("\nNOTE: L3 Bootline saved to NVRAM, reboot to make it take effect\n");
    }
    else
    {
        printf("\nWARNING:Bootline longer than BOOT_LINE_SIZE (%d bytes). Not saved to NVRAM\n", (int) BOOT_LINE_SIZE);
    }

    return OK;
}


STATUS sysSetL2Bootline()
{
    char tmpBuf[BOOT_LINE_SIZE];
    int length;    
    char l2FileName[20] = "/ata0a/l2Bootline";    
    FILE *fdl2;

    strncpy (tmpBuf, (char *)BOOT_LINE_ADRS_L2, BOOT_LINE_SIZE);	
    bootParamsPrompt (tmpBuf);
    length = strlen(tmpBuf);
    if ( length <= BOOT_LINE_SIZE)
    {
       /* update bts config data */
        bspNvRamWrite((char *)BOOT_LINE_ADRS_L2, tmpBuf, length+1);/*add crc, jy080918*/
        /*生成crc，写入l2bootline   jy080918*/
        cfGetCrc((char *)BOOT_LINE_ADRS_L2, 0);
        if ((fdl2 = fopen (l2FileName, "w+")) == (FILE *)ERROR)
        {
            printErr ("\nCannot open when write\"%s\".\n", l2FileName);                
        }
        else
        {
            fwrite( (const void *)BOOT_LINE_ADRS_L2, 1,BOOT_LINE_SIZE, fdl2); 
            fflush(fdl2);
            fclose (fdl2);
        }        
        printf("\nNOTE: L2 Bootline saved to NVRAM, reboot BTS to make it take effect\n");
    }
    else
    {
        printf("\nWARNING:Bootline longer than BOOT_LINE_SIZE (%d bytes). Not saved to NVRAM\n", (int) BOOT_LINE_SIZE);
    }
}


UINT32 bspGetBtsHWVersion()
{
    return 0;
}

UINT16 bspGetBtsHWType()
{
    return 0;
}

/************SENSOR DATA FORMAT**************

D15 	D14 	D13 	D12 	D11 	D10 	  D9		 	D8		 	D7 			D6		 	D5 			D4 			D3 			D2 		D1 		D0 	 
Sign 	Sign 	Sign 	Sign 	MSB 	Bit 7 	Bit 6 	Bit 5 	Bit 4 	Bit 3 	Bit 2 	Bit 1 	Bit 0 	CRIT 	HIGH 	LOW 	 

Temperature data is represented by a 10-bit, 
two’s complement word with an LSB (Least 
Significant Bit) equal to 0.5°C:
	
Temperature  Digital_Output_Binary  Hex
+130°C       01 0000 0100           104h
+125°C       00 1111 1010           0FAh
+25°C        00 0011 0010           032h
+0.5°C       00 0000 0001           001h
0°C          00 0000 0000           000h
-0.5°C       11 1111 1111						3FFh
-25°C        11 1100 1110           3CEh
-55°C        11 1001 0010           392h
*******************************************/
BOOL bspGetTemperatureSupportStatus()
{
#if 0
    T_I2C_TABLE       i2cData;

    if ( 0 != i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (UINT8 *)&i2cData, sizeof(T_I2C_TABLE)))
    {
        return FALSE;
    }
	
    if(i2cData.PCBVersion[7]>= '3') /*  "91-8000X-..., if X is 3 or higher, there is a temperature sensor connected */
    {
		return TRUE;
    }
#endif
    return FALSE;
}


int bspGetBTSTemperature()
{
#if 0
    static BOOL isFirstTime = TRUE;
    static BOOL isSensorOnline = FALSE;
	short temp;
	int result;

    if (isFirstTime)
    {
        isSensorOnline = bspGetTemperatureSupportStatus();
        isFirstTime = FALSE;
    }

    if (isSensorOnline)
    {
        int rc = i2c_read(TEMP_SENSOR_ADDR,0,(char *)&temp,2);
        if ( -1 != rc )
        {
            result = temp >> 3;
            result = result >> 1;  /* *0.5°C *********/
        /*
            printf("BTS temperature:%d\n!  bspGetBTSTemperature!!!!",result);
        */
            return result; 
        }
    }
    else
    {
        return 38;
    }
#endif
   return 38;
}

BOOL bspGetIsRebootBtsWanIfDiscEnabled()
{
    T_NVRAM_BTS_CONFIG_DATA *param;
    param = (T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS;
    if (    NVRAM_VALID_PATTERN != param->rebootBtsWanIfDiscPattern
         || 0 == param->rebootBtsWanIfDisc)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL bspGetPPPoEpermit()
{
    T_NVRAM_BTS_CONFIG_DATA *param;
    param = (T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS;
    if (    NVRAM_VALID_PATTERN != param->permitPPPoEPattern
         || 0 == param->permitPPPoE)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL bspGetIsRebootBtsIfFtpDownEnable()
{
    T_NVRAM_BTS_CONFIG_PARA *param;
    param = (T_NVRAM_BTS_CONFIG_PARA *)NVRAM_BASE_ADDR_PARA_PARAMS;
    if (    NVRAM_VALID_PATTERN != param->rebootBtsIfFtpDownPattern
         || 0 == param->rebootBtsIfFtpDown)
    {
        return FALSE;
    }
    return TRUE;
}
BOOL bspGetIsPermitUseWhenSagDown()
{
    T_NVRAM_BTS_CONFIG_PARA *param;
    param = (T_NVRAM_BTS_CONFIG_PARA *)NVRAM_BASE_ADDR_PARA_PARAMS;
    if (    NVRAM_VALID_PATTERN != param->permitUseWhenSagDownPattern
         || 0 == param->permitUseWhenSagDown)
    {
        return FALSE;
    }
    return TRUE;
}
BOOL bspGetFilterSTP()
{
    T_NVRAM_BTS_CONFIG_DATA *param;
    param = (T_NVRAM_BTS_CONFIG_DATA *)NVRAM_BASE_ADDR_APP_PARAMS;
    if (    NVRAM_VALID_PATTERN != param->filterSTPPattern
         || 0 == param->filterSTP)
    {
        return FALSE;
    }
    return TRUE;
}

unsigned int bspGetCPLDVersion()
{
      return 1;
}

const UINT8 crc_8_tab[256]=  {
0x00,0x2F,0x5E,0x71,0xBC,0x93,0xE2,0xCD,0x57,0x78,
0x09,0x26,0xEB,0xC4,0xB5,0x9A,0xAE,0x81,0xF0,0xDF,
0x12,0x3D,0x4C,0x63,0xF9,0xD6,0xA7,0x88,0x45,0x6A,
0x1B,0x34,0x73,0x5C,0x2D,0x02,0xCF,0xE0,0x91,0xBE,
0x24,0x0B,0x7A,0x55,0x98,0xB7,0xC6,0xE9,0xDD,0xF2,
0x83,0xAC,0x61,0x4E,0x3F,0x10,0x8A,0xA5,0xD4,0xFB,
0x36,0x19,0x68,0x47,0xE6,0xC9,0xB8,0x97,0x5A,0x75,
0x04,0x2B,0xB1,0x9E,0xEF,0xC0,0x0D,0x22,0x53,0x7C,
0x48,0x67,0x16,0x39,0xF4,0xDB,0xAA,0x85,0x1F,0x30,
0x41,0x6E,0xA3,0x8C,0xFD,0xD2,0x95,0xBA,0xCB,0xE4,
0x29,0x06,0x77,0x58,0xC2,0xED,0x9C,0xB3,0x7E,0x51,
0x20,0x0F,0x3B,0x14,0x65,0x4A,0x87,0xA8,0xD9,0xF6,
0x6C,0x43,0x32,0x1D,0xD0,0xFF,0x8E,0xA1,0xE3,0xCC,
0xBD,0x92,0x5F,0x70,0x01,0x2E,0xB4,0x9B,0xEA,0xC5,
0x08,0x27,0x56,0x79,0x4D,0x62,0x13,0x3C,0xF1,0xDE,
0xAF,0x80,0x1A,0x35,0x44,0x6B,0xA6,0x89,0xF8,0xD7,
0x90,0xBF,0xCE,0xE1,0x2C,0x03,0x72,0x5D,0xC7,0xE8,
0x99,0xB6,0x7B,0x54,0x25,0x0A,0x3E,0x11,0x60,0x4F,
0x82,0xAD,0xDC,0xF3,0x69,0x46,0x37,0x18,0xD5,0xFA,
0x8B,0xA4,0x05,0x2A,0x5B,0x74,0xB9,0x96,0xE7,0xC8,
0x52,0x7D,0x0C,0x23,0xEE,0xC1,0xB0,0x9F,0xAB,0x84,
0xF5,0xDA,0x17,0x38,0x49,0x66,0xFC,0xD3,0xA2,0x8D,
0x40,0x6F,0x1E,0x31,0x76,0x59,0x28,0x07,0xCA,0xE5,
0x94,0xBB,0x21,0x0E,0x7F,0x50,0x9D,0xB2,0xC3,0xEC,
0xD8,0xF7,0x86,0xA9,0x64,0x4B,0x3A,0x15,0x8F,0xA0,
0xD1,0xFE,0x33,0x1C,0x6D,0x42};

UINT8 GenerateCRC8for8N(UINT8* DataBuf,UINT16 len)
{
    UINT8 oldcrc8 = 0x00; 
    UINT8 tb; 
    UINT16 charcnt = 0; 
    UINT8 t;//, tmp; 

    while (len--)
    { 
        t = oldcrc8; 
        tb = crc_8_tab[t]; 
        oldcrc8 = DataBuf[charcnt++]^tb; 
    }
    t = oldcrc8; 
    oldcrc8 = crc_8_tab[t]; 
    return oldcrc8;
}
const UINT16 crc_16_tab[256]={ 
0x0000,0x2D17,0x5A2E,0x7739,0xB45C,0x994B,0xEE72,0xC365,
0x45AF,0x68B8,0x1F81,0x3296,0xF1F3,0xDCE4,0xABDD,0x86CA,
0x8B5E,0xA649,0xD170,0xFC67,0x3F02,0x1215,0x652C,0x483B,
0xCEF1,0xE3E6,0x94DF,0xB9C8,0x7AAD,0x57BA,0x2083,0x0D94,
0x3BAB,0x16BC,0x6185,0x4C92,0x8FF7,0xA2E0,0xD5D9,0xF8CE,
0x7E04,0x5313,0x242A,0x093D,0xCA58,0xE74F,0x9076,0xBD61,
0xB0F5,0x9DE2,0xEADB,0xC7CC,0x04A9,0x29BE,0x5E87,0x7390,
0xF55A,0xD84D,0xAF74,0x8263,0x4106,0x6C11,0x1B28,0x363F,
0x7756,0x5A41,0x2D78,0x006F,0xC30A,0xEE1D,0x9924,0xB433,
0x32F9,0x1FEE,0x68D7,0x45C0,0x86A5,0xABB2,0xDC8B,0xF19C,
0xFC08,0xD11F,0xA626,0x8B31,0x4854,0x6543,0x127A,0x3F6D,
0xB9A7,0x94B0,0xE389,0xCE9E,0x0DFB,0x20EC,0x57D5,0x7AC2,
0x4CFD,0x61EA,0x16D3,0x3BC4,0xF8A1,0xD5B6,0xA28F,0x8F98,
0x0952,0x2445,0x537C,0x7E6B,0xBD0E,0x9019,0xE720,0xCA37,
0xC7A3,0xEAB4,0x9D8D,0xB09A,0x73FF,0x5EE8,0x29D1,0x04C6,
0x820C,0xAF1B,0xD822,0xF535,0x3650,0x1B47,0x6C7E,0x4169,
0xEEAC,0xC3BB,0xB482,0x9995,0x5AF0,0x77E7,0x00DE,0x2DC9,
0xAB03,0x8614,0xF12D,0xDC3A,0x1F5F,0x3248,0x4571,0x6866,
0x65F2,0x48E5,0x3FDC,0x12CB,0xD1AE,0xFCB9,0x8B80,0xA697,
0x205D,0x0D4A,0x7A73,0x5764,0x9401,0xB916,0xCE2F,0xE338,
0xD507,0xF810,0x8F29,0xA23E,0x615B,0x4C4C,0x3B75,0x1662,
0x90A8,0xBDBF,0xCA86,0xE791,0x24F4,0x09E3,0x7EDA,0x53CD,
0x5E59,0x734E,0x0477,0x2960,0xEA05,0xC712,0xB02B,0x9D3C,
0x1BF6,0x36E1,0x41D8,0x6CCF,0xAFAA,0x82BD,0xF584,0xD893,
0x99FA,0xB4ED,0xC3D4,0xEEC3,0x2DA6,0x00B1,0x7788,0x5A9F,
0xDC55,0xF142,0x867B,0xAB6C,0x6809,0x451E,0x3227,0x1F30,
0x12A4,0x3FB3,0x488A,0x659D,0xA6F8,0x8BEF,0xFCD6,0xD1C1,
0x570B,0x7A1C,0x0D25,0x2032,0xE357,0xCE40,0xB979,0x946E,
0xA251,0x8F46,0xF87F,0xD568,0x160D,0x3B1A,0x4C23,0x6134,
0xE7FE,0xCAE9,0xBDD0,0x90C7,0x53A2,0x7EB5,0x098C,0x249B,
0x290F,0x0418,0x7321,0x5E36,0x9D53,0xB044,0xC77D,0xEA6A,
0x6CA0,0x41B7,0x368E,0x1B99,0xD8FC,0xF5EB,0x82D2,0xAFC5};


UINT16 bspGenerateCRC16(UINT16* DataBuf, UINT16 len)/*number of bytes in Data buffer;*/
{ 
    UINT16 oldcrc16 = 0x0000; 
    UINT16 tb; 
    UINT16 charcnt = 0;
    UINT8 t; 
    UINT8 tmp;

    while (len--)
    { 
        t= (oldcrc16 >> 8) & 0xFF; /*address to look up table*/
        tb=crc_16_tab[t]; /* LUT*/
        oldcrc16&=0xFF;    /* delete the higher octet.*/

        if ( (charcnt & 1) == 0 ) 
            tmp = DataBuf[charcnt/2]>>8;
        else
            tmp = DataBuf[charcnt/2]&0x00FF;

        oldcrc16= (oldcrc16 << 8) | tmp; /*concatenation */
        oldcrc16=oldcrc16^tb; /*xor operation;*/
        charcnt++;
    }
    /*handle the tail word;*/
    charcnt=2;
    while (charcnt--)
    {
        t= (oldcrc16 >> 8) & 0xFF; /*address t*/
        tb=crc_16_tab[t]; /* LUT*/
        oldcrc16&=0xFF;    /*delete the MSB b*/
        oldcrc16= (oldcrc16 << 8);
        oldcrc16=oldcrc16^tb; /*xor operation;*/
    }
    return oldcrc16;
} 
/*将数据保存到cf卡前要生成crc
*/
char cfGetCrc(char *pBuf0, UINT16 len)
{
    UINT16 crc;    
    char ctemp, ctemp1[6];
    char pBuf[256];

    if((pBuf0 == (char*)NVRAM_BASE_ADDR_APP_PARAMS)||(pBuf0==(char*)NVRAM_BASE_ADDR_PARA_PARAMS))
    {
        memcpy(pBuf, pBuf0, len);
        crc = bspGenerateCRC16((UINT16*)pBuf, len); 
        /*将2字节的crc每4为转为一个字符,加30为防止出现0,存一个字节*/
        pBuf[len] = EOS;
        ctemp = (char)((crc>>12)&0x0f);    
        pBuf[len+1] = ctemp+30;
        ctemp =(char) ((crc>>8)&0x0f);
        pBuf[len+2] = ctemp+30;
        ctemp = (char)((crc>>4)&0x0f);
        pBuf[len+3] = ctemp+30;
        ctemp = (char)(crc&0x0f);
        pBuf[len+4] = ctemp+30;
        pBuf[len+5] = EOS;   
        bspNvRamWrite(pBuf0, pBuf, (len+6));
        return TRUE;
    }
    /*l3bootline or l2bootline*/   
    if(len == 0)/*it is a string.*/
        len = strlen(pBuf0);    
    memcpy(pBuf, pBuf0, len);
    crc = bspGenerateCRC16((UINT16*)pBuf, len);    
    /*将2字节的crc每4为转为一个字符,加30为防止出现0,存一个字节*/
    ctemp1[0] = EOS;
    ctemp1[1] = (char)((crc>>12)&0x0f)+30; 
    ctemp1[2] =(char) ((crc>>8)&0x0f) +30;    
    ctemp1[3] = (char)((crc>>4)&0x0f) +30;    
    ctemp1[4] = (char)(crc&0x0f) +30;    
    ctemp1[5]= EOS;  
    memcpy(pBuf+len, ctemp1, 6);    
    bspNvRamWrite(pBuf0, pBuf, len+6);   
    return TRUE;
}
/*使用cf卡数据前要校验crc
*/
char cfCrc(char *pBuf, UINT16 len)
{
    UINT16 crc; 
    UINT16 temp;

    if(len == 0)
        len = strlen(pBuf);    
    crc = bspGenerateCRC16((UINT16*)pBuf, len);    
    temp = (UINT16)((pBuf[len+1]-30)<<12) + (UINT16)((pBuf[len+2]-30)<<8) 
        + (UINT16)((pBuf[len+3]-30)<<4) + (UINT16)(pBuf[len+4]-30);    
    if(crc == temp)
        return OK;
    else
        return ERROR;
}
UINT32 GetActiveVersion()
{
    if(BOOT_PLANE_A == bspGetBootPlane())
    {    
        return bspGetLoadVersion_A();
    }
    else
    {    
        return bspGetLoadVersion_B();
    }
}
extern unsigned char stop_watchdog;
extern int bootFlag;

void ResetBTSViaCPLD()
{   
     if(bootFlag==0x55)
     {
         return;
     }
     stop_watchdog = 1;
    reboot( BOOT_NO_AUTOBOOT);
}

void  setboolineFlag()
 {
    T_BootLoadState bootState;
  //  int tmpValue;

    bspNvRamRead((char *)(&bootState), (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    if ( bootState.nvramSafe != NVRAM_VALID_PATTERN )
    {
        memset(&bootState, 0, sizeof(bootState));
        bootState.nvramSafe = NVRAM_VALID_PATTERN;
    }

    
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));
 }
void setMac()
{
      #if 1
	//T_I2C_TABLE i2cData;
   int n = 0;
     int i;
   // unsigned short checksum;
  //  unsigned short *buffer; 
  //  unsigned int tempValue;
 //   unsigned int pageNum;


    T_EMAC Mac;
  //  int tmpValue;

    bspNvRamRead((char *)(&Mac), (char *)BOOT_LINE_ADRS_L2, sizeof(Mac));
    if ( Mac.nvramSafe != NVRAM_VALID_PATTERN )
    {
        memset(&Mac, 0, sizeof(Mac));
        Mac.nvramSafe = NVRAM_VALID_PATTERN;
    }

   // i2c_read(I2C_E2PROM_DEV_ADDR, 0x0, (UINT8 *)&i2cData, sizeof(T_I2C_TABLE));

    printf ("\n'.' = clear field;  '-' = go to previous field;  ^D = quit\n\n");

    /* prompt the user for each item;
     *   i:  0 = same field, 1 = next field, -1 = previous field,
     *     -98 = error, -99 = quit
     */

  /*  ioctl (STD_IN, FIOFLUSH, 0 ); */

    FOREVER
    {
        switch ( n )
        {
          

            case 0:
                printf ("L3 MAC              : %02x-%02x-%02x-%02x-%02x-%02x  ",
                         Mac.L3_mac[0], Mac.L3_mac[1], Mac.L3_mac[2],
                         Mac.L3_mac[3], Mac.L3_mac[4], Mac.L3_mac[5]);
                promptEnet(0,Mac.L3_mac);
		  i = 1;
                break;

            case 1:  
                printf ("L2 MAC              : %02x-%02x-%02x-%02x-%02x-%02x  ",
                         Mac.L2_mac[0], Mac.L2_mac[1], Mac.L2_mac[2],
                         Mac.L2_mac[3], Mac.L2_mac[4], Mac.L2_mac[5]);
                promptEnet(1,Mac.L2_mac);
				i =1;
                break;
                
	    /*    case 2:  
                printf ("L1 MAC              : %02x-%02x-%02x-%02x-%02x-%02x  ",
                         Mac.L1_mac[0], Mac.L1_mac[1], Mac.L1_mac[2],
                         Mac.L1_mac[3], Mac.L1_mac[4], Mac.L1_mac[5]);
                promptEnet(Mac.L1_mac);
				i =1;
	
                break;
                */


            default: i = -99; break;
        }

        /* check for QUIT */

        if ( i == -99 )
        {
            printf ("\n");
            break;
        }

        /* move to new field */

        if ( i != -98 ) n += i;

    }/**forever******/



   bspNvRamWrite((char *)BOOT_LINE_ADRS_L2, (char *)&Mac, sizeof(Mac));
#endif

}

/*************************************************
函数名:  bspGetRRUChannelNum
返回值:4或者8



*****************************************************/
unsigned int  bspGetRRUChannelNum()
{
    T_NVRAM_BTS_CONFIG_PARA *param;
    param = (T_NVRAM_BTS_CONFIG_PARA *)NVRAM_BASE_ADDR_PARA_PARAMS;
    if(( param->RRU_Antenna==4)||( param->RRU_Antenna==8))
    	{
    	    return  param->RRU_Antenna;
    	}
	else
	{
	      return 8;
	}
 
}

/*****************************************************
函数名:  bspGetslaveBTSID
返回值:
*****************************************************/
//zengjihan 20120801 for GPSSYNC
unsigned int  bspGetslaveBTSID()
{
    T_NVRAM_BTS_CONFIG_PARA *param;
    param = (T_NVRAM_BTS_CONFIG_PARA *)NVRAM_BASE_ADDR_PARA_PARAMS;
   if(NVRAM_VALID_PATTERN != param->slaveBTSIDFLAG)
    {
      return 0;
    }
    return  param->slaveBTSID;
}
