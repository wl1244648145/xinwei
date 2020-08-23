#include "StatCounter.h"
#include "timeCounter.h"

UINT32 measure_Entity[COUNTER_MAX][DEFAULT_MAX];
UINT16 count_In[COUNTER_MAX];

void IncreaseEntity(timecounter count,bool flag)
{
    if(count_In[count]>=DEFAULT_MAX)
        return;
    UINT32 ticknow=sysGetUSCounter();
    measure_Entity[count][*(count_In+count)]=ticknow-measure_Entity[count][*(count_In+count)];
    if(flag) 
        ++count_In[count];
}

extern "C" void mShow()
{
    UINT16 imax=0,imin=0,i=0;
    UINT32 sumlea=0;
    printf("\r\n%-6s%-10s%-10s%-10s","No:","MaxT:","MinT:","Ave:");

	for(UINT8 t=0;t<COUNTER_MAX;t++)
		{
		imax=0;imin=0;i=0;
		printf("\r\n----------------------------%10s",strFromDir[t]);
		while( i <count_In[t] )
        	{
        	UINT32 tmp=measure_Entity[t][i];
			imax=(tmp>measure_Entity[t][i])?i:imax;
			imin=(tmp<measure_Entity[t][i])?i:imin;
        	++i;
        	}
		if(0==i)
			continue;
    	printf("\r\n%-6d%-10d%-10d%-10d",
           i,
           measure_Ingress[t][imax],measure_Ingress[t][imin],
           sumlea/i);
		}

    printf("\r\n");
    memset(measure_Entity,0,COUNTER_MAX*DEFAULT_MAX*sizeof(UINT32));
    memset(count_In,0,COUNTER_MAX*sizeof(UINT32));
    
}
