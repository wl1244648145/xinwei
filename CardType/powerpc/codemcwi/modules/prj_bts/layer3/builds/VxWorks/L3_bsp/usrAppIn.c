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

extern int main();
extern int reset_BTS();
extern void sysNvDataModifyHandle(); 
extern void check_i2c_eeprom();
extern void StartWdtTask();
extern STATUS bspCreateRamDisk();


void usrAppInit (void)
{

	BOOT_PARAMS       bootParams;
	
    char shellPromp[20];

    /**** add default route ****/
	(void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    if (bootParams.gad[0] != EOS)
	{
		if(routeAdd ("0.0.0.0", bootParams.gad) == ERROR)
		{
			printf("Unable to add route to %s; errno = 0x%x.\n", "0.0.0.0", errno);
		}
	}

    bspTextProtect();    /* protect text memory */

    bspSetSystemTime();    /* initialize and create RTC and set system time */

	rebootHookAdd(reset_BTS);

    StartWdtTask();

/*    taskPrioritySet(taskNameToId("tShell"), 200);  */
    /**********************/


#ifdef	USER_APPL_INIT
	USER_APPL_INIT;		/* for backwards compatibility */
#endif

    taskSuspend(taskNameToId("tShell"));
    sysNvDataModifyHandle();  /* change bts config parameter prompt */
    taskResume(taskNameToId("tShell"));    

	check_i2c_eeprom();

    /* add application specific code here */
    sprintf(shellPromp, "bts(%d) L3->", bspGetBtsID());
    shellPromptSet(shellPromp); 

    bspCreateRamDisk();

}


