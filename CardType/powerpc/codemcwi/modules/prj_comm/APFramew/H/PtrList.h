/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: PtrList.h
 *
 * DESCRIPTION:
 *     PtrList is a high performance general pointer container, which has the interface
 * same as STL's list template, and has high performance implementation similar to microsoft's
 * PtrList class.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 09/04/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/

#ifndef _INC_PTRLIST
#define _INC_PTRLIST

#ifdef __WIN32_SIM__
#ifndef _INC_STDDEF
#include <stddef.h>
#endif
#elif __NUCLEUS__
#include <stddef.h>
#else
#ifndef _STDDEF_H
#include <stddef.h>
#endif
#endif
#include "datatype.h"

#ifndef __NUCLEUS__
#define PTR_LIST_INIT_POOL_SIZE  2500
#define PTR_LIST_POOL_GROW_SIZE  100
#else
#define PTR_LIST_INIT_POOL_SIZE  40
#define ALLOW_POOL_GROW_DYNAMIC
#endif


class CPtrList
{
protected:
    struct CNode;
public:
    class iterator
    {
        friend class CPtrList;
    public:
        iterator():m_pNode(NULL){};
        iterator(CNode* pNode):m_pNode(pNode){};
        inline void*& operator*() {return m_pNode->_Value;};
        inline iterator& operator++() 
        {
            m_pNode = m_pNode->_pNext;
            return *this;
        };
        inline iterator operator++(int)
        {
            CPtrList::iterator _Tmp = *this;
            ++*this;
            return _Tmp;
        }

        inline iterator& operator--()
        {
            m_pNode = m_pNode->_pPrev;
            return *this;
        }

        inline iterator operator--(int)
        {
            CPtrList::iterator _Tmp = *this;
            --*this;
            return _Tmp;
        }
        inline bool operator==(const iterator& rIter) const
        {
            return (m_pNode == rIter.m_pNode);
        };

        inline bool operator!=(const iterator& rIter) const
        {
            return (m_pNode != rIter.m_pNode);
        }
    private:
        CNode* m_pNode;
    };
    friend class iterator;

protected:
    struct CNode
    {
    public:
		#ifdef __NUCLEUS__
		bool   isInStaticPool;
		#endif
        CNode* _pPrev;
        CNode* _pNext;
        void*  _Value;
    };

public:
    CPtrList();
    ~CPtrList();
	static bool InitPtrListClass();

    inline unsigned long size() const { return m_nSize;};
    inline bool empty() const {return m_nSize==0;};

    inline iterator begin() {return iterator(m_pHead->_pNext);};
    inline iterator end(){return iterator(m_pHead);};

    inline void*& front(){return (*begin());};
    void*& back(){ return (*(--end()));};

    inline void push_front(void* value){insert(begin(), value);};
    inline void pop_front(){ erase(begin());};

    inline void push_back(void* value) { insert(end(), value);};
    inline void pop_back(){ erase(--end());};

    iterator insert(iterator _p, void* value);
    iterator erase(iterator _p);
    iterator erase(iterator _F, iterator _L);

    inline void clear(){erase(begin(), end());};
    iterator find(iterator, iterator, void* value);


protected:
    CNode* m_pHead;
    unsigned long m_nSize;
private:
    static CNode* GetNewNode();
    static void   FreeNode(CNode*);

    static CNode*  m_pFirstFreeNode;
	static UINT16  m_usNodeCount;
	static UINT16  m_usFreeNodeCount;

#ifdef __WIN32_SIM__	
    static CMutex *s_pPtrListmutex;
#endif
public:
	static UINT16  m_usNodeCountMax;
};

#endif
