#include "../inc/spinlockapi.h"
static unsigned int spin_trylock(T_SpinLock *lock)
{
	unsigned long tmp, token;

	token = 1;
	__asm__ __volatile__(
"1:	lwarx		%0,0,%2\n\
	cmpwi		0,%0,0\n\
	bne-		2f\n\
	stwcx.		%1,0,%2\n\
	bne-		1b\n\
	isync\n\
2:"	: "=&r" (tmp)
	: "r" (token), "r" (&lock->udLock)
	: "cr0", "memory");

	return tmp;
}

void spin_lock(T_SpinLock *lock)
{
    int timeout = 100000;
	while (1) 
	{
		if (likely(spin_trylock(lock) == 0))
		{
			break;
		}
		do 
		{
		    #if 1
		    if (!timeout--) 
		    {
                        sched_yield();
                        timeout = 100000;
                    }
		    #endif
		} while (unlikely(lock->udLock != 0));
	}
}

void spin_unlock(T_SpinLock *lock)
{
	__asm__ __volatile__("# spin_unlock\n\t"
				"sync" "\n": : :"memory");
	lock->udLock = 0;
}

 
int BspSpinLockInit(T_SpinLock *ptLock)
{
    if(ptLock)
	{
	    ptLock->udLock=0;
	}
	return 0;
}

 
void BspSpinLock (T_SpinLock *ptLock)
{
    TASK_LOCK();/*禁止任务调度*/
    spin_lock(ptLock);
}

 
int BspSpinTryLock (T_SpinLock *ptLock)
{
    return spin_trylock(ptLock);
}

 
void BspSpinLockIrq (T_SpinLock *ptLock, unsigned long* pulFlag)
{
    * pulFlag=INT_LOCK();/*禁止中断*/
    spin_lock(ptLock);
}

 
void BspSpinUnLock (T_SpinLock *ptLock)
{
    spin_unlock(ptLock);
	  TASK_UNLOCK();/*运行任务调度*/
}

 
void BspSpinUnLockIrq (T_SpinLock *ptLock,unsigned long ulFlag)
{
    spin_unlock(ptLock);
	  INT_UNLOCK(ulFlag);/*允许中断*/
}
