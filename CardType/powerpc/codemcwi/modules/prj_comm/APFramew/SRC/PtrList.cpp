#ifndef _INC_PTRLIST
#include "PtrList.h"
#endif

#ifndef __WIN32_SIM__
#ifndef __NUCLEUS__
//#include <intLib.h>//delete by huangjl
#endif
#endif

CPtrList::CNode*  CPtrList::m_pFirstFreeNode = NULL;
UINT16            CPtrList::m_usFreeNodeCount = 0;
UINT16            CPtrList::m_usNodeCount = 0;
UINT16            CPtrList::m_usNodeCountMax = 0;
#ifdef __WIN32_SIM__	
CMutex*           CPtrList::s_pPtrListmutex = NULL;
#endif



#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
bool CPtrList::InitPtrListClass()
{
#ifdef __WIN32_SIM__
    s_pPtrListmutex = new CMutex;
    if (s_pPtrListmutex==NULL)
        return false;
#endif

    m_pFirstFreeNode = (CNode*) new UINT8[sizeof(CNode)*PTR_LIST_INIT_POOL_SIZE];
    if (m_pFirstFreeNode==NULL)
        return false;

    CNode* pNode = m_pFirstFreeNode;
    for (int i = 0; i< PTR_LIST_INIT_POOL_SIZE-1 ;i++)
    {
        pNode->_pNext = pNode+1;
		#ifdef __NUCLEUS__
		pNode->isInStaticPool = true;
		#endif
		pNode++;
    }
	pNode->_pNext = NULL;
	#ifdef __NUCLEUS__
	pNode->isInStaticPool = true;
	#endif
	
	m_usNodeCount = PTR_LIST_INIT_POOL_SIZE;
	m_usNodeCountMax = PTR_LIST_INIT_POOL_SIZE;
	m_usFreeNodeCount = PTR_LIST_INIT_POOL_SIZE;
	
	return true;
}


///////////////////////////////////////////////////////////////
// CPtrList::iterator

///////////////////////////////////////////////////////////////
// CPtrList
CPtrList::CPtrList()
:m_pHead(0),m_nSize(0)
{
    CNode* pNode = GetNewNode();
	if (pNode)
	{
        m_pHead = pNode;
        m_pHead->_pNext = m_pHead->_pPrev = m_pHead;
        m_pHead->_Value = NULL;
	}
	else
	{
	    while(1);
    }
}

CPtrList::~CPtrList()
{
}

CPtrList::CNode* CPtrList::GetNewNode()
{
#ifdef __WIN32_SIM__
    if (!s_pPtrListmutex->Wait())
    {
        LOG(LOG_CRITICAL,0,"Wait s_pmutexlist failed.");
        return NULL;
    }
#elif __NUCLEUS__
    UINT16 oldlevel;
    oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
#else //VxWorks
//    int oldLvl = ::intLock();//delete by huangjl
#endif

    if (m_pFirstFreeNode == NULL) //no node allocated or used up
    {
        #ifndef __NUCLEUS__
	        CNode* pNode = (CNode*)new UINT8[sizeof(CNode)*PTR_LIST_POOL_GROW_SIZE];
	 		if ( pNode )
	 		{
	   		    m_pFirstFreeNode = pNode;
	 		    for (int i = 0; i<PTR_LIST_POOL_GROW_SIZE-1; i++)
	 		    {
	                 pNode->_pNext = pNode+1;
	 		    }
	 			pNode->_pNext = NULL;
				m_usNodeCount += PTR_LIST_POOL_GROW_SIZE;
        		if (m_usNodeCount > m_usNodeCountMax )
		        {
  					m_usNodeCountMax = m_usNodeCount;
				}
	        }
		#else
		    CNode *pNode = NULL;
	  		#ifdef ALLOW_POOL_GROW_DYNAMIC
	  		pNode = new CNode;
	  		if (pNode)
	  		{
	  			pNode->isInStaticPool = false;
			    m_usNodeCount ++;
        		if (m_usNodeCount > m_usNodeCountMax )
		        {
  					m_usNodeCountMax = m_usNodeCount;
				}
	  		}
	  		#endif
            #ifdef __WIN32_SIM__
            if (!s_pPtrListmutex->Release())
            {
                LOG(LOG_CRITICAL,0,"Release s_pmutexlist failed.");
            }
            #elif __NUCLEUS__ 
            ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
            #else //VxWorks
 //           ::intUnlock(oldLvl);//delete by huangjl
            #endif

			return pNode;
			
		#endif

			
    }

    CNode* pNode = m_pFirstFreeNode;
	if (m_pFirstFreeNode)
	{
        m_pFirstFreeNode = m_pFirstFreeNode->_pNext;
        if(m_usFreeNodeCount>0)
        	{
    	m_usFreeNodeCount --;
        	}
	}

	#ifdef __WIN32_SIM__
    if (!s_pPtrListmutex->Release())
    {
        LOG(LOG_CRITICAL,0,"Release s_pmutexlist failed.");
    }
    #elif __NUCLEUS__ 
    ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
    #else //VxWorks
 //   ::intUnlock(oldLvl);//delete by huangjl
    #endif

    return pNode;
}

void CPtrList::FreeNode(CPtrList::CNode* pNode)
{
    if (pNode==NULL)
        return;

#ifdef __WIN32_SIM__
    if (!s_pPtrListmutex->Wait())
    {
        LOG(LOG_CRITICAL,0,"Wait s_pmutexlist failed.");
        return NULL;
    }
#elif __NUCLEUS__
    UINT16 oldlevel;
    oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
#else //VxWorks
//    int oldLvl = ::intLock();//delete by huangjl
#endif

    #ifdef __NUCLEUS__
	if ( !pNode->isInStaticPool )
	{
	    delete pNode;
	    if(m_usNodeCount>0)
	    	{
		m_usNodeCount --;
	    	}
	}
	else
	#endif
	{
 	    pNode->_pNext = m_pFirstFreeNode;
 	    m_pFirstFreeNode = pNode;
    	m_usFreeNodeCount ++;
	}

	#ifdef __WIN32_SIM__
    if (!s_pPtrListmutex->Release())
    {
        LOG(LOG_CRITICAL,0,"Release s_pmutexlist failed.");
    }
    #elif __NUCLEUS__ 
    ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
    #else //VxWorks
 //   ::intUnlock(oldLvl);//delete by huangjl
    #endif
}



CPtrList::iterator CPtrList::insert(CPtrList::iterator _p, void* value)
{
    CNode* pNode = _p.m_pNode;
    CNode * newNode = GetNewNode();
    if ( newNode )
    {
        newNode->_pPrev = pNode->_pPrev;
        newNode->_pNext = pNode;
        pNode->_pPrev = newNode;
        pNode = pNode->_pPrev;
        pNode->_pPrev->_pNext = pNode;
        pNode->_Value = value;
        m_nSize++;
        return CPtrList::iterator(pNode);
    }
    else
        return NULL;
    
}

CPtrList::iterator CPtrList::erase(CPtrList::iterator _p)
{
    CNode* pNode = (_p++).m_pNode;
    pNode->_pPrev->_pNext = pNode->_pNext;
    pNode->_pNext->_pPrev = pNode->_pPrev;
    FreeNode(pNode);
    if(m_nSize!=0)//lijinan 20100121
    	m_nSize--;
    return _p;
}

CPtrList::iterator CPtrList::erase(CPtrList::iterator _F, CPtrList::iterator _L)
{
    while (_F!=_L)
    {
        erase(_F++);
    }
    return _F;
}


CPtrList::iterator CPtrList::find(iterator _F, iterator _L, void* value)
{
    for (;_F!=_L;++_F)
    {
        if (*_F == value)
            break;
    }
    return (_F);
}
