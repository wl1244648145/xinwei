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

#ifndef _INC_L3OAMFTPCLIENT
#define _INC_L3OAMFTPCLIENT

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
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

#ifndef _INC_L3OAMFILELOADNOTIFY
#include "L3OamFileLoadNotify.h"
#endif

/*    |------------------------|
	  |     ���ļ����ܳ���     |
	  |------------------------|
      |   ���ļ��е��ļ�����   |
	  |------------------------|
	  |     �ļ�����ƫ����     |
	  |------------------------|
	  |   �ļ����� |  �ļ���   |
	  |------------------------|
	  |        ........        |
	  |------------------------|
	  |     ���ļ����ݿ�ʼ     |
	  |------------------------|*/  

#define MAX_FILE_CNT            40
#define PACK_FILE_SUCCESS       0
#define PACK_FILE_FAILURE       1
#define RW_FILE_BLOCK           (1000 * 1024) // 1M
#define MAX_FILE_NAME_LEN       32

//�ļ���Ϣ�ṹ
typedef struct
{
	UINT32  FileLen;
    SINT8   FileName[MAX_FILE_NAME_LEN]; 
}TFileInfo;

//���ļ�ͷ�ṹ
typedef struct
{
	char  ActiveVersion[16];
	char  StandbyVersion[16];
	UINT32    PackFileLen;
	UINT32    FileCnt;
	TFileInfo FileInfoA[MAX_FILE_CNT];
}TPackFileHead;


//���͸�ftp client ����Ϣ M_OAM_DL_FILE_NOTIFY
class CTaskFfpClient : public CBizTask
{
public:
    CTaskFfpClient();
    static CTaskFfpClient* GetInstance();    
    int Ftp_GetTid(){ return m_idsys/*m_idFtpC*/; };
    ~CTaskFfpClient(){ 
    	if( DstBuff )    		delete [] DstBuff;
    	if( TemBuff )    		delete [] TemBuff;
    	TemBuff = NULL;
    	DstBuff = NULL;
    	m_Instance = NULL;
    };
private:
    UINT8 *DstBuff;
    UINT8 *TemBuff;
    static CTaskFfpClient * m_Instance;    
private:
	void   Ftp_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result);
	bool   SM_FileDLoadNotify(CFileLoadNotify &rMsg, UINT8);    //֪ͨftp client �ļ�����
    UINT16 SM_FileDLoadExe(CFileLoadNotify&, UINT8);
    bool   SM_UnPackInflateFile(FILE* );
    bool   SM_UnPackFile(FILE* pPackFile, TPackFileHead *PackFileHead);
    bool   FM_InflateFile(TPackFileHead *PackFileHead);
	bool   SM_FTPSendMsg(UINT8 type, UINT16 transid, UINT16 result);
    void   InflateTest();
	void   UploadCalibrationFile(CMessage &);
	UINT8* getFullPathFileName(UINT8 *, UINT8 *);
	bool   FTPCalibrationFile(UINT8 *, UINT8 *);
	bool   SetAttrib( SINT32 plane, const TPackFileHead *PackFileHead, bool bReadOnly ); 
	void   WriteToNvRam();
#ifdef WBBU_CODE
	bool check_rru_file(SINT8 *filename);
#endif
private:
    bool Initialize();
    bool ProcessMessage(CMessage&);
    TID  GetEntityId() const;

    #define FTPC_MAX_BLOCKED_TIME_IN_10ms_TICK (60000)
    bool IsMonitoredForDeadlock()  { return false; };  /* can not enable, FTP server maybe too slow */
    int  GetMaxBlockedTime() { return WAIT_FOREVER ;};

    void InitSysApp(); //��ʼ��Ӧ�ò��߼�
};
#endif
