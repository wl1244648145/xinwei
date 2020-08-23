/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   define the class for L2PciShell
 *                To get rid of the serial port on L2 PPC, start a lineEditor on L3 console,
 *                send the input to L2 PPC tSioRx, receive serial IO output from L2 PPC
 *                tSioTx task and display on the L3 console.
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/04/2006   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef __INC_L2SHELL
#define __INC_L2SHELL

#include "ComEntity.h"
#include "ComMessage.h"
#include "taskdef.h"
#include "L3L2MessageId.h"
//#include "msgQLib.h"
#include "BizTask.h"

#define MAX_CMD_TO_L2_BUF   20
#define MAX_CONSOLE_LINE_LEN  256
#define MAX_MSG_FROM_L2     200

class CL2Shell: public CBizTask
{
public:
    static CL2Shell* GetInstance();
    TID     GetEntityId() const { return M_TID_L2SHELL;};
    void    ShowStatus();
    STATUS  SendCommandToL2(char *bufPtr, int lienLen);
    STATUS  SendControlCommandToL2(bool enable);
    SEM_ID GetL2ShellSEM() const {return SEM_L2ShellCount;};//sunshanggu --080627
    STATUS l2shell(int coreID);
    void  send_Notify_MessageToL2(int coreID, int messageID, char *str);
private:
    
    CL2Shell();
    static CL2Shell *Instance;
    bool   ProcessComMessage(CComMessage* pComMsg);
    bool   Initialize();

    #define L2SHELL_MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return L2SHELL_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    int        TID_L2ShellInput;

    UINT32 CommandToL2Count;
    UINT32 MsgFromL2Count;

    SEM_ID SEM_L2ShellCount; //sunshanggu - 080627
};


#endif
