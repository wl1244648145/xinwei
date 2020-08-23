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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMMODULEBOOTUPNOTIFY
#define _INC_L3OAMMODULEBOOTUPNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

//收到tboot的系统启动指示消息进行此消息处理
//读系统启动标志，若从BTS启动，则模拟EMS按既定顺序发送初始化配置消息到
//配置管理模块，因为配置模块启定时器，因此系统管理模块不用起定时器，收到
//配置管理模块的应答（失败）后，发送下一条配置数据知道所有配置数据发送完；
//若从EMS启动，则向EMS发送数据下载指示，然后进入while语句接受消息。
//对系统bootup指示消息需要进行定义，内容要包括每个模块/cpu的启动结果
//系统启动告警检测定时器开始系统检测
class CModuleBootupNotify : public CMessage
{
public: 
    CModuleBootupNotify(CMessage &rMsg);
    CModuleBootupNotify();
    bool CreateMessage(CComEntity&);
    ~CModuleBootupNotify();
    UINT32 GetDefaultDataLen() const;
    
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);  

private:
#pragma pack(1)
    struct T_BootupInfo
	{
	    UINT16 TransId;
	};
#pragma pack()
};
#endif
