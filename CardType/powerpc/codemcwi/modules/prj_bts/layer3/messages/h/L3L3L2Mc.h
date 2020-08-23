/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L3L2MCPSTATENOTIFY
#define _INC_L3L3L2MCPSTATENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

class CL3L2McpStateNoitfy : public CMessage
{
public: 
    CL3L2McpStateNoitfy(CMessage &rMsg);
    CL3L2McpStateNoitfy();
    bool CreateMessage(CComEntity&);
    ~CL3L2McpStateNoitfy();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    const  T_MCPStateInfo* GetMCPStateInfo()const; 
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_MCPStateInfo MCPStateInfo;
    };
#pragma pack()
};
#ifdef WBBU_CODE
class CL3L2Core9StateNoitfy : public CMessage
{
public: 
    CL3L2Core9StateNoitfy(CMessage &rMsg);
    CL3L2Core9StateNoitfy();
    bool CreateMessage(CComEntity&);
    ~CL3L2Core9StateNoitfy();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    const  T_Core9StateInfo* GetCORE9StateInfo()const; 
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_Core9StateInfo CORE9StateInfo;
    };
#pragma pack()
};

class CL3L2AifStateNoitfy : public CMessage
{
public: 
    CL3L2AifStateNoitfy(CMessage &rMsg);
    CL3L2AifStateNoitfy();
    bool CreateMessage(CComEntity&);
    ~CL3L2AifStateNoitfy();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    const  T_AifStateInfo* GetAifStateInfo()const; 
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        T_AifStateInfo Aifinfo;
    };
#pragma pack()
};
#endif
#endif
