/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    DAC.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   11/29/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_DAC_H__
#define __DATA_DAC_H__

#include "BizTask.h"
#include "ComMessage.h"
#include "log.h"

#include "L3DataTypes.h"

//DAC任务参数定义
#define M_TASK_DAC_TASKNAME      "tDAC"
#ifdef __WIN32_SIM__
#define M_TASK_DAC_OPTION        (0x0008)
#define M_TASK_DAC_MSGOPTION     (0x02)
#else
#define M_TASK_DAC_OPTION        ( VX_FP_TASK )
#define M_TASK_DAC_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_DAC_STACKSIZE     (20480)
#define M_TASK_DAC_MAXMSG        (1024)


//DAC task
class CTaskDAC : public CBizTask
{
public:
    static CTaskDAC* GetInstance();

#ifdef UNITEST
    /*UNITEST时,所有的成员函数都是Public*/
public:
#else
private:
#endif
    CTaskDAC();
    ~CTaskDAC(){}

    bool ProcessMessage(CMessage&);
    inline TID GetEntityId() const 
        {
        return M_TID_L2MAIN;
        }
    inline bool IsNeedTransaction()
        {
        return false;
        }

    void swap16(UINT8*, UINT32);

private:

    //static members:
    static CTaskDAC* s_ptaskDAC;
};

#endif /*__DATA_DAC_H__*/
