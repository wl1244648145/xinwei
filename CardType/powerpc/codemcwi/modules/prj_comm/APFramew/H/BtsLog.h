/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: BtsLog.h
 *
 * DESCRIPTION:
 *     Declaration of FWKLIB's Log Utility.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 10/30/2005  Liu Qun     Added CPE log support interface.
 * 09/04/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_BTS_LOG
#define _INC_BTS_LOG


#include "lstLib.h" //list lib modify by huangjl
#include <time.h>
//#include <msgQLib.h> //modify by huangjl
#include "Vxw_hdrs.h"
#include "ComMessage.h"
#include "datatype.h"

#define MAX_LOG_QUEUE_DEPTH  2000
#ifdef M_TGT_L3 
#ifndef WBBU_CODE
#define NEW_TELNET  //sunshanggu for telnet output
#endif
#endif

#ifdef NEW_TELNET
#define printf TelnetPrintf
extern "C" int TelnetPrintf(const char *_format, ...);
#endif

/*---------------------------------------------------------------*/
//CEidTBL类的设计为LOG 添加了选择性输出(根据eid) 的功能
#define EIDTBL_DEPTH    20
#define ALL_OPEN        0
#define ALL_CLOSE       1
#define SELECTED        2
#define EMPTY_EID       0

class CEidTBL    
{
    short     m_Status;           //0:全开, 1:全关, 2:参照下表显示LOG
    short     m_Index;           //存放查找结果
    UINT32    m_EidTable[EIDTBL_DEPTH]; //目前是小数组,可以更改为大链表
    //List<CEidNode> m_EidList[EIDTBL_DEPTH];
    
    void ClearTbl();    
        
public:
    CEidTBL();
    bool Find(UINT32 eid);
    void Add (UINT32 eid);
    void Del (UINT32 eid);
    void Show();
	
};
/*---------------------------------------------------------------*/

class CLog
{
public:
    void LogDebugMsg(const char* lpszFileName,int nLine, LOGLEVEL level, UINT32 errcode, const char *text,
                       int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, UINT32 eid=0);//modified by maqiang
    void LogComMessage(const char* lpszFileName, int nLine, LOGLEVEL level, UINT32 errcode, const CComMessage* pComMsg, const char* info);
    void LogDbgString(LOGLEVEL level, UINT32 code, const char* text, int arg1, int arg2, int arg3, int arg4);
    static CLog* GetInstance();
    bool StartLogTask();
    CLog();
    CEidTBL EidTbl4LOG;

private:

    typedef struct
    {
        NODE        lstNode;
        char        text[MAX_STRING_LEN + sizeof('\0')];
    } LOG_NODE;

    typedef enum
    {
        LOG_STRING,
        LOG_TEXT_MSG,
        LOG_COM_MSG
    }LOG_TYPE;

    typedef struct 
    {
        LOG_TYPE           type;
        LOG_NODE*          node;            // Ptr to Text
        UINT8*             comMsgData;     // ptr to comMsg data
        int                tid;             // Task Id
        time_t             logTime;         // Time
        char const*        fileName;        // File Name
        UINT32             line;            // Line Num
        UINT16             errCode;         // Err Code
        int                arg0;
        int                arg1;
        int                arg2;
        int                arg3;
        int                arg4;//added by maqiang
        int                arg5;//added by maqiang
    }PACKED T_LogMsg;

    static void _LogMain();
    static CLog* Instance;

    CLog(CLog&);
    void LogMain();
    void DisplayMsg(T_LogMsg& msg, char* buff);

    LIST        FreeLogNodeList;
    MSG_Q_ID    LogMsgQueId;
    INT32       LogErrorCount;

};

#ifdef M_TGT_L2
UINT32  getEid();

#define _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4 ,arg5,arg6) \
              CLog::GetInstance()->LogDebugMsg(__FILE__, __LINE__, \
			  logLevel, errorCode, text, arg1, arg2, arg3, arg4 ,arg5,arg6, getEid())
#else
#define _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4 ,arg5,arg6) \
              CLog::GetInstance()->LogDebugMsg(__FILE__, __LINE__, \
			  logLevel, errorCode, text, arg1, arg2, arg3, arg4 ,arg5,arg6)
#endif			 
#define LOG(logLevel, errorCode, text)                          _LOG(logLevel, errorCode, text, 0,0,0,0,0,0)
#define LOG1(logLevel, errorCode, text, arg1)                   _LOG(logLevel, errorCode, text, arg1, 0,    0,    0,0,0)
#define LOG2(logLevel, errorCode, text, arg1,arg2)              _LOG(logLevel, errorCode, text, arg1, arg2, 0,    0,0,0)
#define LOG3(logLevel, errorCode, text, arg1, arg2, arg3)       _LOG(logLevel, errorCode, text, arg1, arg2, arg3, 0,0,0)
#define LOG4(logLevel, errorCode, text, arg1, arg2, arg3, arg4) _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4,0,0)
#define LOG5(logLevel, errorCode, text, arg1, arg2, arg3, arg4,arg5) _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4,arg5,0)
#define LOG6(logLevel, errorCode, text, arg1, arg2, arg3, arg4,arg5,arg6) _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4,arg5,arg6)


#define _LOG_STR(logLevel, code, text, arg1, arg2, arg3, arg4) \
              CLog::GetInstance()->LogDbgString(logLevel, code, text, arg1, arg2, arg3, arg4) 
#define LOG_STR(logLevel, code,  text)                            _LOG_STR(logLevel,code, text, 0,    0,    0,    0)
#define LOG_STR1(logLevel, code, text, arg1)                      _LOG_STR(logLevel,code, text, arg1, 0,    0,    0)
#define LOG_STR2(logLevel, code, text, arg1, arg2)                _LOG_STR(logLevel,code, text, arg1, arg2, 0,    0)
#define LOG_STR3(logLevel, code, text, arg1, arg2, arg3)          _LOG_STR(logLevel,code, text, arg1, arg2, arg3, 0)
#define LOG_STR4(logLevel, code, text, arg1, arg2, arg3, arg4)    _LOG_STR(logLevel,code,  text, arg1, arg2, arg3, arg4)

#define LOGMSG(logLevel, errorCode, pComMsg, text)\
         CLog::GetInstance()->LogComMessage(__FILE__, __LINE__, logLevel, errorCode, pComMsg, text)


#endif //_INC_BTS_LOG
