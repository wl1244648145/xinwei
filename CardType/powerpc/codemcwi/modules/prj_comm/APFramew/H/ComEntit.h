/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   define the class for CComEntity -- application modules that 
 *                 communicate with each other through CComMessage
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   5/11/2005    Qun Liu      Initial file creation.
 *
 *   3/22/2006   Yushu Shi     Add message log enable feature
 *---------------------------------------------------------------------------*/

#ifndef _INC_COMENTITY
#define _INC_COMENTITY

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifndef _INC_TID
#include "taskdef.h"
#endif

#ifndef _INC_PTRLIST
#include "PtrList.h"
#endif

class CComMessage;

#ifdef __WIN32_SIM__
class CPersonator;
#endif

#define M_COMENTITY_LOGMSG 0x00000001

#ifdef __WIN32_SIM__
#define M_COMMSG_POOL_INIT_SIZE 2000
#define M_COMMSG_POOL_GROW_SIZE 100
#elif __NUCLEUS__
#ifdef BF_NU_L2
#define M_COMMSG_POOL_INIT_SIZE 200
#else
#define M_COMMSG_POOL_INIT_SIZE 20
#endif
#define M_COMMSG_POOL_GROW_SIZE 10
#else //Vxworks
#ifdef M_TGT_L3
#define M_COMMSG_POOL_INIT_SIZE 80000 //70000   // 60000 for EB downlink packets, 20000 for uplink and others
#else
#define M_COMMSG_POOL_INIT_SIZE 20000
#endif
#define M_COMMSG_POOL_GROW_SIZE 100
#endif

#ifdef __NUCLEUS__
#ifdef PCMCIA_ARM
#define M_COMMSG_POOL_MAX_NUM   120
#else
#ifndef BF_NU_L2
#define M_COMMSG_POOL_MAX_NUM   160
#else
#define M_COMMSG_POOL_MAX_NUM   600
#endif
#endif
#endif

#ifndef __NUCLEUS__
//#define M_MONITOR_RESOURCE
#endif
#ifdef M_MONITOR_RESOURCE
typedef enum _Enum_RES_TYPE
{
	RES_MONITOR_TIMER=0,
	RES_MONITOR_COMMSG,

	RES_MONITOR_MAX
}Enum_RES_TYPE;
typedef enum _ENUM_RES_OP
{
	RES_OP_ALLOC=0,
	RES_OP_FREE,

	RES_OP_MAX
}ENUM_RES_OP;
typedef struct _TaskResCntT
{
	UINT32 nCnt[RES_OP_MAX];
}TaskResCntT;
typedef struct _ResCntT
{
	UINT32 nPoolSize;
	UINT32 nInUseMAX;
	UINT32 nInUse;
//	UINT32 nFree;
}ResCntT;
typedef struct _ResStatCntT
{
	ResCntT res_Cnt[RES_MONITOR_MAX];
	TaskResCntT res_Task_Cnt[RES_MONITOR_MAX][M_TID_MAX];
}ResStatCntT;
extern ResStatCntT g_ResMonitor;
TID getMsgCreatorTID(CComMessage* pComMsg);
void updateResPoolSize(Enum_RES_TYPE type, UINT32 size);
void addResPool(Enum_RES_TYPE type, int added);
bool updateResStat(Enum_RES_TYPE type, ENUM_RES_OP opType, int count, TID tid);

#endif //#ifdef M_MONITOR_RESOURCE

class CComEntity  
#ifndef NDEBUG 
:public CObject
#endif
{
private:
    typedef struct _EntityEntry
    #ifndef __NUCLEUS__
    {
        CComEntity* pEntity;
        UINT32 idsys;
       _EntityEntry():pEntity(NULL),idsys(0xFFFFFFFF){}
    }EntityEntry;
	#else
	{
		CComEntity* pEntity;
		_EntityEntry():pEntity(NULL){}
	}EntityEntry;
	#endif

private:

#ifdef __WIN32_SIM__
    static EntityEntry s_EntityTable[M_TID_MAX+1];
    static CMutex *s_pmutexlist;
    /************************
     *多任务的showStatus()总是
     *交互着打印，影响使用
     *增加一个互斥信号量
     *-by xiaoweifang
     */
    static CMutex s_mutexShowStatus;
#else
    static EntityEntry s_EntityTable[M_TID_MAX];
#endif

    static CComMessage *s_pFreeComMsg;
    static bool s_bClassInited;

    static UINT32 MaxAllocatedCnt;
	static UINT32 PoolSize;

protected:
    UINT32 m_idsys;

public:
	static UINT32 CurrentAllocatedCnt;

    static bool InitComEntityClass();

    static bool PostEntityMessage(CComMessage* pComMsg, SINT32 timeout=NO_WAIT, bool isUrgent=false);

    #ifndef __NUCLEUS__
	static CComEntity* getEntity( TID tid );
    static void StatShow();
    #endif
     void setEntityNull(TID tid);
#ifdef __WIN32_SIM__
    static TID LookupTid(UINT32 idsys);
    /************************
     *ShowStatus()调用前WAIT;
     *调用后务必调用RELEASE
     *-by xiaoweifang
     */
    static bool WAIT()     { return s_mutexShowStatus.Wait(); }
    static bool RELEASE()  { return s_mutexShowStatus.Release(); }
#endif

    virtual bool PostMessage(CComMessage*, SINT32, bool isUrgent=false)=0;

    virtual TID GetEntityId() const =0;

    //Low Level Allocation
    virtual void* Allocate(size_t &size);
    virtual bool Deallocate(void*, size_t);

    //High Level
    #ifdef __NUCLEUS__
    void* AllocateComMessage(size_t &size, size_t &DataSize, bool canBeDiscarded = false);
	#else
    void* AllocateComMessage(size_t &size, size_t &DataSize);
	#endif
	
    virtual bool DeallocateComMessage(CComMessage* pComMsg);

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

protected:
    CComEntity();
    bool RegisterEntity(bool bIdOnly);

    //virtual ~CComEntity();

private:
    CComEntity(CComEntity&);

#ifdef __WIN32_SIM__
    friend class CPersonator;
#endif

};


#endif
