//***************************************************************************************
// ��ʱ���ܲ�ѯ(����ͷ): L3 -> 6�ֽ�
//                       L2 -> 14�ֽ�
//                       UT -> 10�ֽ�
//
//***************************************************************************************






#ifndef _INC_L3_BTS_PM
#define _INC_L3_BTS_PM

#ifdef __WIN32_SIM__
#include <Winsock2.h>
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <vxWorks.h>
#include <ioLib.h>
#include <ftpLib.h>
#include <inetLib.h>
#include <usrLib.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#endif
#include <stdio.h>

#ifndef _INC_L3_BTS_MESSAGEID
#include "L3_BTS_PMMessageID.h"
#endif

#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

//#ifndef _INC_L3OAMFILE
//#include "L3OamFile.h"
//#endif

#include <time.h>
#include <stdio.h>
#include "mcWill_bts.h"
#include <string.h>
extern "C" SINT32 bspGetBtsID();
extern void GetDataPerfData ( UINT8 Type, UINT8 *pData );
extern void GetOAMPerfData  ( UINT8 *pData );
extern void GetVoicePerfData( UINT8 *pData );

#define _MYDEBUG            //   ***   ***   ***   TEST-TEST   ***   ***   ***
//#define _CPEPM_TEST
//#define BTSPM_L2_TEST
//#define TEST_GET_L2PERFDATA

#ifndef WBBU_CODE
#define    SM_CPE_PERF_DIR     "/RAMD/cpe/"
#else
#define    SM_CPE_PERF_DIR     "/RAMD:0/cpe/"
#endif
#define    SM_CPE_SIG_DIR     "/ata0a/sig/"
const    UINT16    BTSPM_ENT_BTS_L2    =    0x01;            //�豸���Ͳ��L2
const    UINT16    BTSPM_ENT_BTS_L3    =    0x02;            //�豸���Ͳ��L3
const    UINT16    BTSPM_ENT_RTMONITOR =    0x03;            //�豸���Ͳ��L3
const    UINT16    BTSPM_ENT_CPE       =    0x10;            //�豸���Ͳ��CPE
const UINT16 BTSPM_ENT_MEM = 0x11;//�豸���Ͳ��MEM
const    UINT16    ERROR_CODE          =    0xFFFF;            //Ϊ����
const    UINT16    RTMONITOR_TRANSID_SEND    = 0x80E9;
const    UINT16    RTMONITOR_TRANSID_RECV    = 0x0A;
#ifdef _MYDEBUG
const    UINT32    L1L2CPE_REQUEST_TIMEOUT    =    5000;            //L1L2CP������������ʱ
#else
const    UINT32    L1L2CPE_REQUEST_TIMEOUT    =    300000;            //L1L2CP������������ʱ
#endif

const    UINT16    PERIOD_TRANSACTIONID    =    0x7AAA;            //���������ݵ�transactionID
const    UINT16    L3_BTS_PM_ABSTRACT_FILENAME_LEN = 100;
#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

UINT32 GetSystemTime();
UINT32 GetCpeReportTime();

//for Log Config
typedef struct T_BTSPM_FTPINFO   {
    UINT16 uPortNum;            //Perf server Port Num
    UINT16 uRqtInterval;        //Report Interval
    UINT16 uCollectInterval;    //Collecting Interval
    UINT32 uIPAddr;                //Perf Server IP address
    UINT8  UserLen;                //UserName length
    UINT8  PassLen;                //PassWord length
    char chUserName[10];        //Perf username
    char chPassWord[10];        //Perf password
};
//signal report head
#pragma pack(2)
    typedef struct{
    UINT32 headLen;
    UINT16 version_type;
    UINT32 Eid;
}SIG_FILE_HEAD;
    typedef struct {   
        UINT32 timeHead;
        UINT32  uEndTime;
        UINT32 DataLen;
        UINT32 flameNum;
        UINT32 seqNum;
        UINT16 version;
        UINT16 type;
        UINT16 len;
        UINT16 rev;
    } SIGNAL_HEAD;
#pragma pack()
class CTaskPM : public CBizTask
{
enum{ RTMntActTm=0, RTMntStartTm, RTMntDelayTm, RTMntMaxTm};

private:
	CTimer*  m_ptmArrRTMnt[RTMntMaxTm];
//	UINT16   m_usTransId;
	UINT8*   m_pucDelayData[2];
	UINT16   m_pucDelayDataLen[2];
	UINT8     m_curren_Index_put;
	UINT8     m_curren_Index_get;
	UINT16   m_usGlobalIdx;
	bool      StartRTMonitorActive(UINT16 usTmLen);
	CTimer*  InitRTMonitorTimer(  bool bPeriod, UINT16 usMsgID, UINT16 usPeriod );
	UINT32   GetRTMonitorTimeStamp(UINT8* uc);
	void     PM_RTMonitorReq(CMessage& rMsg );
	void     PM_StartRTMonitor(CMessage& rMsg );
	UINT16   GetTRMonitorTimerDelay();
public:
	UINT16   m_usRTMonitorTmLen;
	bool     m_bRTMonitorAct;
	void     InitRTMonitor();
	void     PM_StartRTMonitor( );	
	void reStartPeriodRpt();
	bool DelPerfFile();
       //new stat
       bool DelPerfFile_new();
public:
	void    test();
	UINT32  testGetSec();
public:
    CTaskPM ();
    static CTaskPM* GetInstance(); 
    int PM_GetTid(){ return m_idsys; };
    ~CTaskPM();
private:
    bool Initialize();
    bool ProcessMessage(CMessage& rMsg);
    TID GetEntityId() const;

    #define PM_MAX_BLOCKED_TIME_IN_10ms_TICK (6000)    //60 seconds
    bool IsMonitoredForDeadlock()  { return false; };
    int  GetMaxBlockedTime() { return PM_MAX_BLOCKED_TIME_IN_10ms_TICK ;};


private:
    bool InitPMApp();
    bool InitTimer( UINT8 const nIdx,        //m_TimerInfo�����е����
                    UINT16 const usMsgId ); //MessageID
private:
    static CTaskPM * m_Instance;  
    UINT32 m_uBTSId;
    CTimer* m_pReportTm;
    CTimer* m_pCollectTm;
    CTimer* m_pWaitTM;/*��֤�ɼ�ʱ�����ϱ����ڱ����ĵȴ�ʱ��jy080923*/
    CTimer* m_pWaitColTm;/*�ϱ��ӳٶ�ʱ��*/
    bool m_bPeriodCollect;
    SIGNAL_HEAD m_signalHead;
    UINT16 m_TransationID;
    
    struct{
        char CPESignalFile[100];
        bool isCpeSigFileExist; 
    }m_CpeSignalFileInfo;
    struct {
        T_UTPerfData* pUTPerfData;
        T_L2PerfDataEle* pL2PerfData;
        T_L3PerfData* pL3PerfData;
    } m_tRsrvPerfData;


    struct    {
        bool isBTSFileExist;        //�Ƿ����BTS�ļ�����һ��д������ʱ��true,�����ļ���ʱ��false
        bool isCPEFileExist;        //�Ƿ����CPE�ļ�����һ��д������ʱ��true,�����ļ���ʱ��false
        char BTSPerfFile[30];
        char BTSRsvPerfFile[30];    //FTP���ϱ������ļ�
        char CPEPerfFile[30];
        char CPERsvPerfFile[30];    //FTP���ϱ������ļ�
    } m_tFileInfo;

    enum { /*L1, */L2_PERIOD_TIMER, /*CPE_PERIOD_TIMER,*/ L2_CUR_TIMER, CPE_CUR_TIMER, MAX_TIMER };
    struct    T_L1L2CPE_TIMER
    {
        CTimer *pTimer;
        bool bTimerStart;    //��ʱ���Ƿ�����
        //bool bPeriod;        //�Ƿ����ڲ�ѯ��
        UINT16 uTransID;    //����Ϣ��TransID
        UINT16 uType;        //L2 ��Ϣʹ��
        UINT32 uEID;        //L2, CPE��ʹ�õ�
    };
    T_L1L2CPE_TIMER m_TimerInfo[MAX_TIMER];

#pragma pack(2)
    struct    _tagRECORD_HEAD{
        UINT32 BTSId;
        UINT16 EntType;
        UINT16 EntIndex;
        UINT16 Data_Type;
        UINT32 uStartTime;
        UINT32 uEndTime;
        UINT16 DataLen;
    } m_tRcdHead;
#pragma pack()
/*�µĻ���ͳ����ؽṹ��*/
struct    
{
    bool isBTSFileExist;        //�Ƿ����BTS�ļ�����һ��д������ʱ��true,�����ļ���ʱ��false        
    char BTSPerfFile[40];
    char BTSRsvPerfFile[40];    //FTP���ϱ������ļ�        
} m_tFileInfo_new;
#pragma pack(2)
struct    _tagRECORD_HEAD_NEW
{
    UINT32 BTSId;
    UINT32 uStartTime;
    UINT32 uEndTime;
    UINT32 version;//�汾����ʼ�汾Ϊ0x00
    UINT32 rev; //��ȫ0
    UINT32 length;//�������ݾ��ɳ���    
} m_tRcdHead_new;
#pragma pack()

private:
    void PM_PerfLogCfgReq( CMessage& rMsg );
    void PM_L2PerfDataQueryReq( CMessage& rMsg );      
    void PM_L3PerfDataQueryReq( CMessage& rMsg );        
    void PM_CPEPerfDataQueryReq( CMessage& rMsg ); 
    void PM_L2PerfDataRsp( CMessage& rMsg );
    void PM_CPEPerfDataRsp( CMessage& rMsg );
    void PM_PerfDataQueryTimer();        
    void PM_PerfDataFileUploadTimer();
    bool PM_UploadTimerDealFile( bool isBTSFile, char* scFileName, UINT8 ucFileNameLen );
    void PM_BtsUserReq( CMessage& rMsg );
    void PM_BtsUserRsp( CMessage& rMsg );
    void PM_CPEReportSignal( CMessage& rMsg );//����mem�ϱ�����
    void PM_SignalDataFileUploadTimer();
    void sendSignalDataFile();
    void PM_MakeCpeSignalFile();
    void GetFTPInfo( T_BTSPM_FTPINFO& rFtpInfo, CMessage& rMsg );
    bool StartReportTm();
    bool StartCollectTm();
    bool StartWaitColTm();
    bool StartWaitTm();
    bool MakePerfFileInfo();
    bool SendPerfFile( char* chFileName );    
    bool RemoveFile( char* chFileName );
    bool WriteToFile( bool bBTS,                //[ 0|1 ]�Ƿ���д��BTS�ļ�
                      UINT16 uEntType,        //[BTSPM_ENT_BTS_L1 | BTSPM_ENT_BTS_L2 | BTSPM_ENT_BTS_L3 | BTSPM_ENT_CPE] �豸���Ͳ��
                      unsigned char* chData,    //д�������
                      unsigned int uDataLen );//д�����ݵĳ���
    void GetFileTime( char * chVal );

    void SendMessage(    UINT16 uMsgID,            //MessageID
                         TID uDstTid,            //DestinationID
                         unsigned int uDataLen,    //DataLength
                         UINT8* pData );            //data
    void SetTimerInfo(    bool bTimeoutNotify,    //�Ƿ�ʱ֪ͨ
                          UINT16& uTransID,        //TransID
                          UINT8 uIndex,            //m_TimerInfo[]�е����
                          bool bPeriod );            //������

    void GetL3PerfData( T_L3PerfData* pPerfData );
    void GetCrntPerfData( UINT32* ulRsvPerfData,    //����Ļ�׼ֵ
                          UINT32* ulStscPerfData,    //�ɼ���ֵ����ɺ��γɼ�¼ֵ
                          UINT32 const usLen );        //���ݳ���
    char* GetAbstractFile( char* chFileName, char* chFullName )
    {
        memset( chFullName, 0, L3_BTS_PM_ABSTRACT_FILENAME_LEN );
        strcpy( chFullName, SM_CPE_PERF_DIR );
        //strcat( chFullName, "/" );
        strcat( chFullName, chFileName );
        return chFullName;
    }
    char* GetAbstractSigFile( char* chFileName, char* chFullName )
    {
        memset( chFullName, 0, L3_BTS_PM_ABSTRACT_FILENAME_LEN );
        strcpy( chFullName, SM_CPE_SIG_DIR );
        //strcat( chFullName, "/" );
        strcat( chFullName, chFileName );
        return chFullName;
    }
//  #ifdef _MYDEBUG
//      UINT32 m_ulL2Tmp;
//      UINT32 m_ulL3Tmp;
//  #endif
    
    //new stat
    T_L3PerfData_new l3PerfData_new;
    bool MakePerfFileInfo_new();    
    void PM_PerfDataFileUploadTimer_new();
    bool SendPerfFile_new( char* chFileName );
    bool PM_UploadTimerDealFile_new( bool isBTSFile, char* scFileName, UINT8 ucFileNameLen );
    bool WriteToFile_new(unsigned char* chData,unsigned int uDataLen);
    void GetL3PerfData_new();
    void PM_PerfDataQueryTimer_new();
    void PM_L2PerfDataRsp_new( CMessage& rMsg );
    void PM_ClearDataNew();
};

#endif
