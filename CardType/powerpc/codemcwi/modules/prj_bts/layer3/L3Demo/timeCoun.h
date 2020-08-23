#include "StatCounter.h"

#define DEFAULT_MAX (1000)
#define M_DISC_STRLEN    (10)

typedef enum 
{
    counter0 = 0,
    counter1,
    counter2,
    counter3,
    COUNTER_MAX
}timecounter;

const UINT8 strFromDir[COUNTER_MAX][M_DISC_STRLEN] = {
    "Ingress",       
    "eThNet",      
    "Engress",     
    "Buffer"      
};
void IncreaseEntity(timecounter count,bool flag);
extern UINT32 measure_Entity[COUNTER_MAX][DEFAULT_MAX];
extern UINT16 count_In[COUNTER_MAX];

