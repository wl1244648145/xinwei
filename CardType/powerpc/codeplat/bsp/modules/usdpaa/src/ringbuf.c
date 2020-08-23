#include <stdio.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>


#include "../../../com_inc/bsp_usdpaa_ext.h"
Queue *pQueue;
pthread_mutex_t  g_recv_flag = PTHREAD_MUTEX_INITIALIZER;
Queue *InitQueue(void)
{
    Queue *q;
	int i=0;
	q = (Queue *)malloc(sizeof(Queue));
	if (q == NULL)
		return NULL;
	q->front = 0;
	q->rear = 0;
	q->tag =0;
    memset(q->sizelen,0,MAXSIZE);
	for(i=0;i<MAXSIZE;i++)
	{
	    q->qu[i]=(unsigned int)malloc(MAX_BUF_LEN * sizeof(unsigned char));
		if (q->qu[i] == 0)
		{
			return NULL;
		}
	}
    return q;
}

int eqicnt=0;
int eqicntexit=0;
int enqueuecnt=0;
int dequeuecnt=0;

int EnQueue(Queue *q,unsigned char *dwaddr,int len)
{
#if 0
    if (q->rear == q->front && q->tag == 1)
    {
        //eqicntexit++;
	    return 0;
    }
	else
	{
	    //eqicnt++;
		memcpy((void *)q->qu[q->rear],(void *)dwaddr,len);
		q->sizelen[q->rear] = len;
		//printf("EnQu ok,address->0x%lx!\n",q->qu[q->rear]);
		q->rear = (q->rear+1)%MAXSIZE;
		if (q->rear == q->front)
		{
		    //printf("1-q->rear:0x%lx\n",q->rear);
		    pthread_mutex_lock(&g_recv_flag);
			q->tag = 1;
			pthread_mutex_unlock(&g_recv_flag);
		}
		return 1;
	}
#else
    eqicnt++;
    memcpy((void *)q->qu[q->rear],(void *)dwaddr,len);
    q->sizelen[q->rear] = len;
    q->rear = (q->rear+1)%MAXSIZE;
    if (q->rear == 0)
    {
	    enqueuecnt++;
    }
    return 1;
#endif
}

int dqicnt=0;
int dqicntexit=0;

int DeQueue(Queue *q,unsigned int *plen)
{
#if 0
    int ret=0;
    if (q->rear == q->front && q->tag == 0)
    {
        //dqicntexit++;
		return 0;
    }
    else
    {
       // printf("DeQu ok!\n");
        //dqicnt++;
		ret = q->qu[q->front];
		*plen = q->sizelen[q->front];
		q->front = (q->front+1)%MAXSIZE;
		if (q->rear == q->front)
		{
		    //printf("2-q->rear:0x%lx\n",q->rear);
		    pthread_mutex_lock(&g_recv_flag);
		    q->tag = 0;
			pthread_mutex_unlock(&g_recv_flag);
		}
		return ret;
    }
#else
    int ret=0;
    dqicnt++;
    if (q->rear != q->front || dequeuecnt != enqueuecnt)
    {
		ret = q->qu[q->front];
		*plen = q->sizelen[q->front];
		q->front = (q->front+1)%MAXSIZE;
		if (q->front == 0)
		{
			dequeuecnt++;	
		}
    }
	return ret;

#endif
}

void Display(void)
{
#if 0
    int n,i;
	
    if(pQueue->rear == pQueue->front && pQueue->tag ==1)
        n = MAXSIZE;
    else
        n = (q->rear-q->front + MAXSIZE)%MAXSIZE;
    for(i=0;i<n;i++)
        printf("%4d",q->qu[(q->front+i)%MAXSIZE]);
    printf("\n");
#endif
    //printf("%4d",q->qu[(q->front+i)%MAXSIZE]);
    printf("pQueue->front->0x%lx\n",(unsigned long)pQueue->front);
    printf("pQueue->rear->0x%lx\n",(unsigned long)pQueue->rear);
    printf("pQueue->tag->0x%lx\n",(unsigned long)pQueue->tag);
}

void buftest()
{
    int icnt=0;
    Queue *q;
	unsigned char x[8*1024];
	int len;
	int ret=0;
	memset(x,0,sizeof(x));
	q=InitQueue();
	for (icnt=0;icnt<50;icnt++)
	{
	    if(!EnQueue(q,x,1024))
		    printf("\r\n qu overfull!\n");
	}
	for (icnt=0;icnt<150;icnt++)
	{
		#if 1
		ret = DeQueue(q,(unsigned int *)&len);
	    if(!ret)
		    printf("empty!\n");
		else
		    printf("ret value->0x%lx,len->0x%lx\n",(unsigned long)ret,(unsigned long)len);
		#endif
	}	
}
