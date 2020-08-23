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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/
#pragma warning (disable : 4786)
#ifndef INC_OAML3ALM
#define INC_OAML3ALM

#ifdef __WIN32_SIM__
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <map>
using namespace std;
#include <list>

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif


#ifndef _INC_L3OAMALMNOTIFYOAM
#include "L3OamAlmNotifyOam.h"
#endif

#ifndef _INC_L3OAMALMNOTIFYEMS
#include "L3OamAlmNotifyEms.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#ifndef _INC_L3OODELETEALARMTIFY
#include "L3OODeleteAlarmNotify.h"
#endif

//枚举值对应了m_arrCard[ANTENNA_NUM]的一个bit, 
//最大允许32个此类告警
typedef enum
{
#if 0
    E_ALM_RF_BOARD_VOLTAGE_MINOR,
    E_ALM_RF_BOARD_VOLTAGE_SERIOUS,
    E_ALM_RF_BOARD_CURRENT_MINOR,
    E_ALM_RF_BOARD_CURRENT_SERIOUS,
    E_ALM_RF_TTA_VOLTAGE_MINOR,
    E_ALM_RF_TTA_VOLTAGE_SERIOUS,
    E_ALM_RF_TTA_CURRENT_MINOR,
    E_ALM_RF_TTA_CURRENT_SERIOUS,
    E_ALM_RF_TX_POWER_MINOR,
    E_ALM_RF_TX_POWER_SERIOUS,
    E_ALM_RF_RF_DISABLED,
    E_ALM_RF_BOARD_SSP_CHKSUM_ERROR,
    E_ALM_RF_BOARD_RF_CHKSUM_ERROR,
    E_ALM_RF_BOARD_RF_NORESPONSE,
#endif
    E_ALM_RF_MAX,
    /*之后是一些可能引起RF开关的告警种类*/
    E_ALM_TCXO_FREQOFF = E_ALM_RF_MAX,
    E_ALM_PLL_LOSE_LOCK,
    E_ALM_GPS_SIGNAL,
    E_ALM_GPS_LOC_CLOCK,
    E_ALM_GPS_LOST,
    E_ALM_MAX = 32  /*不允许超过32个*/
} E_AlmType;


class CRFAlmState
{
public:
    CRFAlmState():m_bRFMaskModified(false)
        {
        memset(m_arrCard, 0, sizeof(m_arrCard));
        }
    ~CRFAlmState(){}
    bool setRFAlarm(UINT8 ucAntennaIdx, E_AlmType type);
    bool setAllRFAlarm(E_AlmType type);
    bool clearRFAlarm(UINT8 ucAntennaIdx, E_AlmType type);
    bool clearAllRFAlarm(E_AlmType type);
    bool isRFMaskModified() //是否RF mask 需要更新到L1
        {
        return m_bRFMaskModified;
        }
    void RFMaskRefresh()    //RF mask 已经更新到L1
        {
        m_bRFMaskModified = false;
        }
    UINT16 getRFMask();
private:
    //记录RF是否产生告警，每个bit对应一个告警类型
    UINT32  m_arrCard[ANTENNA_NUM];
    bool    m_bRFMaskModified;
};


struct T_AlarmHandle
{
    SINT32  slKeepAlarmSeconds;//
    bool    bPostProcessFlag;
    CAlarmNotifyOam *pAlarmNotify;
    T_AlarmHandle(CAlarmNotifyOam *pInAlmNotify) 
                  :slKeepAlarmSeconds(M_ALM_MAX_ENDURE_SEC_DEF),pAlarmNotify (pInAlmNotify),bPostProcessFlag(false)
    {}

    ~T_AlarmHandle()
    {
        if (NULL != pAlarmNotify)
        {
            /*pAlarmNotify是clone出来的*/
            pAlarmNotify->DeleteMessage();
            delete pAlarmNotify;
            pAlarmNotify = NULL;
        }
    }
};


//定时器时长:milliseconds.
#define M_ALARM_INTERVAL_KEEP_CLEAR     (1000)
#define M_ALARM_INTERVAL_KEEP_NOT_CLEAR (1000)

//清除告警的保留观察时间(3秒).
#define M_CLEAR_ALM_KEEP_TIME			(3000)

/********************************
 *ALARMTimer: Alarm任务有2种定时器:
 ********************************/
typedef enum 
{
    /*
     *告警消除后不是立即发往EMS,而是观察3秒,如果3秒内
     *没有同样的告警产生，则认为告警消除了
     */
    E_TIMER_ALARM_KEEP_CLEAR = 0,
    /*对特定告警，如L2PPC通信断，
     *需要判断5秒没有恢复
     *的话，重起L2PPC.
     */
    E_TIMER_ALARM_KEEP_NOT_CLEAR,
    E_TIMER_ALARM_MAX
}ALARMTimer;

/********************************
 *stAlarmTimerExpire: 定时器超时消息
 *消息格式,由Alarm发往Alarm任务
 ********************************/
typedef struct _tag_stAlarmTimerExpire
{
    ALARMTimer type;
} stAlarmTimerExpire;


class CTaskAlm : public CBizTask
{
friend void l3oamshowalarm();
friend void l3oamresetsyn(UINT8);
public:
    CTaskAlm();
    struct T_AlmBaseTab
    {
        UINT16  usAlmID;
        SINT32  slSecondsKeepNotClear;    //告警产生后没有恢复的时间，到期后将进行异常处理
    };
    static CTaskAlm* GetInstance();
    const UINT16 getCurrentRFMask();
	const UINT16 getDisplayRFMask();
private:
    static CTaskAlm * m_Instance;    
private:
    bool  Initialize();
    void  MainLoop();
    bool  ProcessMessage(CMessage&);
    TID   GetEntityId() const;

    #define ALARM_MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return ALARM_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

////bool  BufferThisAlarm(CAlarmNotifyOam&); // 告警平滑处理及针对不同告警级别的告警处理    
	bool  AlarmAnalysis(CAlarmNotifyOam&);
	bool  searchInAlarmList(list<T_AlarmHandle*> &Almlist, CAlarmNotifyOam &inAlarm, list<T_AlarmHandle*>::iterator &it);
    bool  getActiveAlmNotify();
    bool  sendActiveAlmToEMS(CAlarmNotifyOam &);
    bool  setAlarmHandler(T_AlarmHandle *);
    bool  clearAlarmHandler(CAlarmNotifyOam &Alarm);
    public:
    bool  sendAlarmHandleMsg(UINT16 MsgId, TID, SINT8* pData = NULL, UINT16 Len = 0);
    private:
    CTimer* InitTimer(ALARMTimer);
    void  AlarmTimeOut(CMessage &);
    void  ProcessClearedAlarm();
////bool  HandleAlarm(T_AlarmHandle *);
    void  CheckAlarmKeepNotClear();
    SINT32 getAlarmTimeByID(UINT16);
    bool  deleteAlarmNotify(CDeleteAlarmNotify& Notify);
    bool  configSYNCPower(UINT16);
    bool  isHandleException()
        {
        return m_bHandleException;
        }
private:
    static T_AlmBaseTab  m_AlmBaseTab[MAX_ALM_TPYE];
    list<T_AlarmHandle*> m_CurAlmList;
////list<T_AlarmHandle*> m_listBufferedAlarm;
	list<T_AlarmHandle*> m_listClearedAlarm;
    UINT32 m_AlmSeqID;
    bool   m_bHandleException;
    CTimer *m_pAlarmKeepClearTimer;
    CTimer *m_pAlarmKeepNotClearTimer;
    map<UINT16, UINT32> m_mapAlmID_KeepTime;
    CRFAlmState         m_RFAlmState;
    UINT8  m_ucRFDisabledMask;
};

#endif
