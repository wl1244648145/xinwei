/**************************************************
*file: cpuUsage.c
*Author: dingfojin
*date: 2006.6.1
*
**************************************************/
#include <stdio.h>
#include <semaphore.h>
#include <semLib.h>
#include <time.h>

int cpuUsagePrint = 0;

int baseCount= 0;
int runCount = 0;
int cpuUsage = 0;
int runTime = 0;
SEM_ID cpuSemID;


void timerFun(int arg){ 
		double temp;
		
    switch(runTime){	   

		case 0:         
  		   runTime = 1; 
/*		   taskPrioritySet(taskNameToId("tCpuUsage"), 2); */
		   break;

		case 1:
		   baseCount = runCount;
		   runTime = 2;
		   taskPrioritySet(taskNameToId("tCpuUsage"), 255); 
		  /* printf("baseCount = %d\n",baseCount);  */
		   break;

		case 2:
		   temp = runCount/baseCount;
		   cpuUsage = 100 - 100*temp;
			if(cpuUsagePrint){
				printf("cpu idle ::%d::, baseCount = %d, runCount = %d,temp:%f\n",cpuUsage,baseCount, runCount,temp);
			}		   
		   break;

		default:
			break;				  
                
	}
    
	/*runCount = 0; */
    cpuSemID->semCount = 0;	
    semGive(cpuSemID);

    return;
}

int getCpuUsage(void){

/*	printf("******CPU Usage = %d *********\n",OspCpuUsage); */

	return cpuUsage;
}



void cpuUsageTask(){

	timer_t timerID;
    int state;
    struct itimerspec iValue;
    struct itimerspec oValue;
	
	cpuSemID = semCCreate(SEM_Q_PRIORITY,1);

    state = timer_create(CLOCK_REALTIME,NULL,&timerID);
    if(state != 0 ){
		printf("Timer create Failure\n");
		return;
	}
    state = timer_connect(timerID, timerFun, 0);
    if(state != 0 ){
		printf("Timer connect Failure\n");
		return;
	}

	iValue.it_interval.tv_nsec =0;
    iValue.it_interval.tv_sec =1;
    iValue.it_value.tv_nsec =0;
    iValue.it_value.tv_sec =10;
    state = timer_settime(timerID,TIMER_ABSTIME,(struct itimerspec*)&iValue,&oValue);
    if(&oValue == NULL){
    	printf("Timer settime Failure\n");
		return;
    }


	/*********************/
	
	while(1){

	   semTake(cpuSemID,WAIT_FOREVER);
	   runCount = 0;

	   while(cpuSemID->semCount == 0){
			runCount++;
	   } 
	}

}

