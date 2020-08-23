#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect((x),0)
#define barrier() __asm__ __volatile__("sync": : :"memory")
#define INT_LOCK()   (0) 
#define INT_UNLOCK(x) 
#define TASK_LOCK()    
#define TASK_UNLOCK(x) 
typedef struct
{
    volatile unsigned long udLock;
}T_SpinLock;

int BspSpinLockInit (T_SpinLock *ptLock);
void BspSpinLock (T_SpinLock *ptLock);
int BspSpinTryLock (T_SpinLock *ptLock);
void BspSpinLockIrq (T_SpinLock *ptLock, unsigned long* pulFlag);
void BspSpinUnLock (T_SpinLock *ptLock);
void BspSpinUnLockIrq (T_SpinLock *ptLock,unsigned long ulFlag);
#endif
