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

#ifdef __WIN32_SIM__
#include <Winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdio.h>
#else
#include <vxWorks.h>
#include <stdio.h>
#include <ioLib.h>
#include <usrlib.h>
#include <ftpLib.h>
#include <string.h>
#include <inetLib.h>
#include <dirent.h>
#include "mcWill_bts.h"
#endif

#ifndef _INC_L3OAMFTPCLIENT
#include "L3OamFtpClient.h"
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_L3OAMTEST
//#include "L3OamTest.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
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

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif

#ifndef _INC_ERRORCODEDEF
#include "ErrorCodeDef.h"
#endif

#ifndef _INC_L3OAMFILE
#include "L3OamFile.h"
#ifdef WBBU_CODE
#include "l3bootTask.h"
#endif
#endif

//extern "C" void bspEnableWriteCF();
//extern "C" void bspDisableWriteCF();

//#define	_LJFDEBUG
//#define   TEST_TIMER   0xEEEE
extern "C" int inflate ( unsigned char * src, unsigned char *   dest, int nBytes);
CTaskFfpClient* CTaskFfpClient:: m_Instance = NULL;
CTaskFfpClient :: CTaskFfpClient()
{
    strcpy(m_szName, "tFtpC");
    m_uPriority   = M_TP_L3FTPCLIENT;
    m_uOptions    = 0;
    m_uStackSize  = SIZE_KBYTE * 50;
    m_iMsgQMax    = 100; 
    m_iMsgQOption = 0;
}


bool CTaskFfpClient :: Initialize()
{
    CBizTask :: Initialize();

    #ifdef __WIN32_SIM__
        #ifdef L3FILE_DEBUG
        InflateTest();
        #endif
    #endif

	
#ifdef	_LJFDEBUG
	CComMessage* pMsg = new ( this, 0 ) CComMessage;
	pMsg->SetDstTid( M_TID_FTPCLIENT);
	pMsg->SetMessageId( TEST_TIMER );
	CTimer* pTimer = new CTimer( true, 20000,  pMsg );
	//pTimer->Start();
#endif

#ifndef WBBU_CODE
    #define DECOMP_BUF_SIZE  (1024 * 1024 * 8) 
    #define COMP_BUF_SIZE    (1024*1024*4)//(DECOMP_BUF_SIZE / 3) 
#else
    #define DECOMP_BUF_SIZE  (1024 * 1024 * 22) //(1024 * 1024 * 6) 
    #define COMP_BUF_SIZE    (1024*1024*15)//(DECOMP_BUF_SIZE / 3) 
#endif
    DstBuff = new UINT8[DECOMP_BUF_SIZE];
    if( ! DstBuff )
    {
        OAM_LOGSTR( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] allocate memory DstBuff error"  );
        return false;
    }
    TemBuff = new UINT8[COMP_BUF_SIZE];
    if( ! TemBuff )
    {
        delete [] DstBuff;
        OAM_LOGSTR( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] allocate memory TmeBuff error"  );
        return false;
    }
    return true;
}

    
bool CTaskFfpClient:: ProcessMessage(CMessage& rMsg)
{
    UINT8  Type;
    UINT16 MsgId;
    
    MsgId = rMsg.GetMessageId();
    switch(MsgId)
    {
        case M_EMS_BTS_DL_BTS_SW_REQ:
            {
            OAM_LOGSTR(LOG_DEBUG3, L3SM_ERROR_REV_MSG, "[tFtpC] Receive from EMS, download BTS software request");
            CFileLoadNotify rNotify(rMsg);
            Type = FILE_BTS_CODE_IMG; 
            SM_FileDLoadNotify(rNotify, Type);
            }
            break;
        
        case M_EMS_BTS_DL_UT_SW_REQ_NEW:
            {
            OAM_LOGSTR(LOG_DEBUG3, L3SM_ERROR_REV_MSG, "[tFtpC] Receive from EMS, download CPE software request");
            CFileLoadNotify rNotify(rMsg);
            Type = FILE_CPE_CODE_IMG; 
            SM_FileDLoadNotify(rNotify, Type);
            }
            break;
        case M_OAM_CFG_FTP_TRANSFER_FILE_REQ:
            OAM_LOGSTR(LOG_DEBUG3, L3SM_ERROR_REV_MSG, "[tFtpC] Receive from tCFG, upload calibration file request");
            UploadCalibrationFile(rMsg);
            break;
        default:
    #ifndef __WIN32_SIM__
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_REV_ERR_MSG, "[tFtpC] receive error msg = 0x%04x");
    #endif
            return true;
    }
    
    return true;
}

TID CTaskFfpClient :: GetEntityId() const
{
   return M_TID_FTPCLIENT;
}

CTaskFfpClient* CTaskFfpClient :: GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskFfpClient;
    }

    return m_Instance;
}

void  CTaskFfpClient :: Ftp_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result)
{
    CL3OamCommonRsp CommonRsp;
    CommonRsp.CreateMessage(*this);
    CommonRsp.SetDstTid(tid);
    CommonRsp.SetTransactionId(transid);
    CommonRsp.SetMessageId(msgid);
    CommonRsp.SetResult(result);
    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3FM_ERROR_REV_MSG, "l3oam ftp task post msg[0x%04x] fail", CommonRsp.GetMessageId());
        CommonRsp.DeleteMessage();
    }
}

bool CTaskFfpClient :: SM_FileDLoadNotify(CFileLoadNotify& rMsg, UINT8 FileType)
{ 
    UINT16 Result = SM_FileDLoadExe(rMsg, FileType); 
/*    Ftp_PostCommonRsp(M_TID_EMSAGENTTX, 
                     rMsg.GetTransactionId(), 
                     M_BTS_EMS_UPGRADE_UT_SW_RSP, 
                     Result);
*/
    SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), Result);
    //启动定时器保护ftp任务jy081121
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
    taskSM->setFtpUsingFlag(M_TID_SM, FALSE);
    return true;
}


UINT16 CTaskFfpClient :: SM_FileDLoadExe(CFileLoadNotify& rMsg, UINT8 FileType)
{
    SINT32 Result = 0;
#ifndef __WIN32_SIM__
    FILE* pFile;

    SINT8 UserName[USER_NAME_LEN];
    memset(UserName, 0, sizeof(UserName));
    rMsg.GetUserName(UserName);

    SINT8 PassWord[USER_PASSWORD_LEN];
    memset(PassWord, 0, sizeof(PassWord));
    rMsg.GetFtpPass(PassWord); 
  
    SINT8 FtpDir[FILE_DIRECTORY_LEN];    
    memset(FtpDir, 0, sizeof(FtpDir));
    rMsg.GetFtpDir(FtpDir);
    
   
    SINT8 FileName[FILE_NAME_LEN];
    memset(FileName, 0, sizeof(FileName));
    rMsg.GetFileName(FileName);

    struct in_addr Svriaddr;
    Svriaddr.s_addr = rMsg.GetFtpServerIp();
    SINT8 IpAddr[16];
    memset(IpAddr, 0, sizeof(IpAddr));
    strcpy(IpAddr, inet_ntoa(Svriaddr));
    
    SINT32 ctrlSock = 0;
    SINT32 dataSock = 0;  
  //  UINT16 ErrCode= 0;


    struct in_addr tmpIpAddr;
     tmpIpAddr.s_addr = htonl(Svriaddr.s_addr);
     SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
     inet_ntoa_b( tmpIpAddr, strIpAddr );
     OAM_LOGSTR3(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] IP:%s, Username:%s, Possword:%s",(int)strIpAddr,(int)UserName,(int)PassWord);
     OAM_LOGSTR2(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Path:%s %s",(int)FtpDir,(int)FileName);
 
    if (ftpXfer (IpAddr, UserName, PassWord, NULL,  "RETR %s",  
        FtpDir, FileName, &ctrlSock, &dataSock) == ERROR)
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] BTS ftpxfer() error");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER);
        return (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER;
    }

    OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] BTS ftpxfer() success");
    if(FILE_BTS_CODE_IMG == FileType)  //将BTS放到备份目录进行解包，解压缩。
    {
        if(ERROR == chdir( SM_BTS_TEMP_DIR))
        {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Chdir(/RAMD/temp/) error");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_FILE_NOT_EXIST);
        return (UINT16)L3SM_ERROR_FILE_NOT_EXIST;
        }
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir(/RAMD/temp/) success");
    }
    else  //Z 、CPE软件不需要解包、解压，直接放到工作目录 
    {
        DIR* pdir = opendir( SM_CPE_BOOT_DIR );
        if( NULL == pdir )
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] opendir( /ata0a/cpe/ ) error");
            if( OK != mkdir( SM_CPE_BOOT_DIR ) )
            {
                OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] mkdir( /ata0a/cpe/ ) error");
//                SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)0x0F0F);
                return 0x0F0F;//创建路径失败
            }
        }
        else
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] opendir( /ata0a/cpe/ ) success");
            if(OK != closedir( pdir ))
            {
                OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] closedir() error");
//                SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_OPEN_FILE);
                return (UINT16)L3SM_ERROR_OPEN_FILE;
            }
        }
        
        if(OK != chdir( SM_CPE_BOOT_DIR ))
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir( /ata0a/cpe/ ) error");
//            SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_OPEN_FILE);
            return (UINT16)L3SM_ERROR_OPEN_FILE;
        }

        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir( /ata0a/cpe/ ) success");

    }
    
    pFile = fopen(FileName, "wb");
    if(NULL == pFile)
    {
        Result = (UINT16)L3SM_ERROR_OPEN_FILE;
        close (dataSock);
        if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
        {
//            SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER);
            return (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER;
        }
        if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
        {
//            SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER);
            return (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER;
        }
        if(ERROR == close (ctrlSock))
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] close (ctrlsock) error");
//            SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
            return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
        }

        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] open file error");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)Result);
        return Result;
    }

    OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fopen( ) success");

//    bspEnableWriteCF();

    #define BUFF_SIZE (1024)
    SINT8 *Tembuf = new SINT8[BUFF_SIZE];
    memset(Tembuf, 0, BUFF_SIZE);
    SINT32 nBytes = 0, nFileLen = 0, nBytesw = 0;

    while ((nBytes = ::read (dataSock, Tembuf, BUFF_SIZE)) > 0)
    {
        //::taskLock(); ////////////////////////////////////////////
        nBytesw = fwrite(Tembuf, 1, nBytes, pFile);
        //::taskUnlock(); ////////////////////////////////////////////
        memset(Tembuf, 0, BUFF_SIZE);
        nFileLen = nFileLen + nBytes;
          
        if(nBytesw != nBytes)
        {
            OAM_LOGSTR3(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fwrite( ) error, read[%d] from datasock but fwrite[%d], totallen[%d]", nBytes, nBytesw, nFileLen);
//            SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
            if(EOF == fclose(pFile))
            {
                OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fclose(pfile) error");
//                SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
                delete []Tembuf;
                return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
            }
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fwrite(pfile) error");
//            SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
            delete []Tembuf;
            return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
        }
    }
    delete []Tembuf;
    if( -1 == nBytes )
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] download file failed, return[-1]");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER);
        return L3SM_ERROR_CANT_LINKTO_SERVER;
    }
    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] write file size = %d", nFileLen);    
    if(EOF == fclose(pFile))
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fclose(pfile) error");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
        return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
    }
    if(ERROR == close (dataSock))
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] close (datasock) error");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)Result);
        return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
    }
    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
    {
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER);
        return (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER;
    }
    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
    {
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER);
        return (UINT16)L3SM_ERROR_CANT_LINKTO_SERVER;
    }

    if(ERROR == close (ctrlSock))
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] close (ctrlsock) error");
//        SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
        return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
    }
    OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] file download success");

    if(FILE_BTS_CODE_IMG == FileType)
    {
 	    SM_FTPSendMsg( FileType, rMsg.GetTransactionId(), L3SM_BTSDL_SUCCESS_1 );
        pFile = fopen(FileName, "rb");
        if(NULL == pFile)
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fopen() error");
//			SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
            return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
        }
        
        
        if(true != SM_UnPackInflateFile(pFile))              //解包同时将文件解压缩
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] sm_unpackinflatefile() error");
//			SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
#ifdef WBBU_CODE
            fclose(pFile);
           remove(FileName);
#endif
            return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
        }
        

        if(ERROR == fclose(pFile))
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] close (ctrlsock) error");
//			SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), (UINT16)L3SM_ERROR_CODE_FILE_ERR);
            return (UINT16)L3SM_ERROR_CODE_FILE_ERR;
        }
	#ifdef WBBU_CODE
	if(OK == remove(FileName))
        {
             
           // OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] remove(%s) Success", (int)FileName);
            printf("[tFtpC] remove(%s) Success\n ", FileName);
            
        }
	#endif
//		SM_FTPSendMsg(FileType, rMsg.GetTransactionId(), L3SM_BTSDL_SUCCESS_2);
		return L3SM_BTSDL_SUCCESS_2;
    }
	else
	    return L3SM_CPEDL_SUCCESS;
#endif

    return Result;
}

bool CTaskFfpClient :: SM_FTPSendMsg(UINT8 type, UINT16 transid, UINT16 result)
{
    CL3OamCommonRsp rNotify;
    rNotify.CreateMessage(*this);
    rNotify.SetDstTid(M_TID_SM);
    rNotify.SetTransactionId(transid);
    if(FILE_BTS_CODE_IMG == type)
        rNotify.SetMessageId(M_BTS_EMS_DL_BTS_SW_RESULT_NOTIFY );
    else if( FILE_CPE_CODE_IMG == type )
        rNotify.SetMessageId( M_BTS_EMS_DL_UT_SW_RESULT_NOTIFY );
    rNotify.SetResult(result);   
    if(true != rNotify.Post())
    {
        #ifndef __WIN32_SIM__
        OAM_LOGSTR1(LOG_DEBUG, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] post msg[0x%04x] fail", rNotify.GetMessageId());
        #endif
        rNotify.DeleteMessage();
    }

    return true;
}

//解包的工作是在目录SM_BTS_TEMP_DIR进行的，解包结束后要将对应的文件解压到
//"/RAMDISK/WORK/"目录，同时将SM_BTS_TEMP_DIR目录下的文件删除
bool CTaskFfpClient :: SM_UnPackInflateFile(FILE* pPackFile)
{
    if(NULL == pPackFile)
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] parameter error");
        return false;
    }
    
    #ifndef __WIN32_SIM__
    if(ERROR == chdir(SM_BTS_TEMP_DIR))  //设置当前工作路径
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir(s/RAMD/temp/) error");
        return false;
    }
    #endif

    TPackFileHead PackFileHead; //文件包信息，供解包、解压、删除同名文件使用
    memset(&PackFileHead, 0, sizeof(PackFileHead));
    bool Result = true;
    Result = SM_UnPackFile(pPackFile, &PackFileHead);      //文件解包 ###########################################
    //return true;
    if(false == Result)
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] sm_unpackfile() error");
        return false;
    }

    Result = FM_InflateFile(&PackFileHead);   //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    if(false == Result)
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fm_inflatefile() error");
        return false;
    }
    
#ifndef __WIN32_SIM__
    BOOL bok = true;
    if(ERROR == chdir(SM_BTS_TEMP_DIR))                   //设置当前工作路径
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir(/RAMD/temp/) error");
        bok = false;
    }
#endif

    #if 1
    for(UINT32 i = 0; i < PackFileHead.FileCnt; i++) //删除目录"/RAMDISK/BACKUP/"下的文件
    {   
        const char *pfilename = (const char *)(PackFileHead.FileInfoA[i].FileName);
        if(0 == strlen(pfilename))
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] filename error");
            bok = false;
        }
        
        if(ERROR == remove(pfilename))
        {
            OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] remove(%s) error", (UINT32)pfilename);
            bok = false;
        }
    }
    #endif
    if( bok )
        OAM_LOGSTR1( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] remove files(%s) success", (int)SM_BTS_TEMP_DIR );

    return true;
}


bool CTaskFfpClient :: SM_UnPackFile(FILE* pPackFile, TPackFileHead *pPackFileHead)
{
    char *lpBuffer = new char[RW_FILE_BLOCK];
    unsigned int  ReadCnt = 0;
    unsigned int  WriteCnt = 0;
    unsigned int CurOffSet = sizeof(TPackFileHead);
    unsigned int i;
  // FILE *fpCfgFile = NULL;
    unsigned int nSize;
 //   char *pFileName = NULL;
    memset(lpBuffer, 0, RW_FILE_BLOCK);
    fseek(pPackFile, 0, SEEK_SET);
    ReadCnt = fread((char *)pPackFileHead, sizeof(TPackFileHead), 1, pPackFile);
    fseek(pPackFile, sizeof(TPackFileHead), SEEK_SET);
    UINT FileCnt = ntohl(pPackFileHead->FileCnt);

    if(MAX_FILE_CNT < FileCnt) 
    {
        delete []lpBuffer;
        return false;
         
    }

//    OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Unpack File Count = [%d]", FileCnt);

    for(i = 0; i < FileCnt; i++)
    {
		taskDelay(50);

        //获取源文件大小
        UINT nFilesize = ntohl(pPackFileHead->FileInfoA[i].FileLen) ;
        FILE *pCurFile = NULL;

        char *pFileName = (char *)(pPackFileHead->FileInfoA[i].FileName);
        pCurFile = fopen((const char *)pFileName, "wb+");
        if(NULL == pCurFile)
        {
            delete []lpBuffer;
             OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Get File Length = 0", (UINT32)pFileName);
            return false;
        }

        //开始拷贝
        UINT  WriteOffSet = 0;
        while(nFilesize > 0)
        {
            nSize = RW_FILE_BLOCK;
            if(nSize > nFilesize)
            {
                nSize = nFilesize;
            }

            memset(lpBuffer, 0, RW_FILE_BLOCK);
            fseek(pPackFile, CurOffSet,   SEEK_SET);
            fseek(pCurFile,  WriteOffSet, SEEK_SET);

            ReadCnt = fread(lpBuffer, nSize, 1, pPackFile);
            WriteCnt = fwrite(lpBuffer, nSize, 1, pCurFile);
        
            CurOffSet   += nSize;
            WriteOffSet += nSize;
        
            nFilesize -= nSize;
        }

        if(0 != fclose(pCurFile))
        {
             OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] fclose(%s) failed", (UINT32)pFileName);
             delete []lpBuffer;
             return false;
        }
//        else
//        {
//             OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] unpack(%s) success", (UINT32)pFileName);
//        }
//        taskdelay(10);
    }
    OAM_LOGSTR1(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] unpack (%02d) Files  success", i);
    
    delete []lpBuffer;    
    return true;
}
#ifdef WBBU_CODE
bool CTaskFfpClient :: check_rru_file(SINT8 *filename)
{
  //  LOG(LOG_CRITICAL, 0, "check_rru_file!!!\n");
    if((strstr(filename, "wrru_fpga"))||(strstr(filename, "wrru_mcu")))
    {
    //  LOG(LOG_CRITICAL, 0, "check_rru_file!!! true!!!\n");
	return true;
    }
    else
    {
    //  LOG(LOG_CRITICAL, 0, "check_rru_file!!!false!!!\n");
	return false;
    }
}
#endif
bool CTaskFfpClient :: FM_InflateFile(TPackFileHead *PackFileHead)
{
    UINT32 FileCnt = ntohl(PackFileHead->FileCnt);
    
    SINT32 plane = 1 - bspGetBootPlane();
    if(BOOT_PLANE_B == plane)
    {
        #ifndef __WIN32_SIM__
        if(ERROR == chdir(SM_BTSB_BOOT_DIR))  //设置当前工作路径 
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir(sm_btsb_boot_dir) error");
            return false;
        }
        #endif
    }
    else
    {
        #ifndef __WIN32_SIM__
        if(ERROR == chdir(SM_BTSA_BOOT_DIR))  //设置当前工作路径 
        {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] chdir(sm_btsa_boot_dir) error");
            return false;
        }
        #endif          
    }	
    OAM_LOGSTR1( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Try to inflate to plane[%d]", plane );
	
    if( ! SetAttrib( plane, PackFileHead, false ) )
        return false;
        
    char ucPrintInfo[20];
    memset( ucPrintInfo, 0, 20 );
    strcpy( ucPrintInfo, (plane==0) ? "/ata0a/btsA" : "/ata0a/btsB" );
    chdir( ucPrintInfo );
    for(UINT32 i = 0; i < FileCnt; i++)
    {
        //获取源文件大小
        #ifndef __WIN32_SIM__
        chdir(SM_BTS_TEMP_DIR);  //设置当前工作路径
        #endif
        FILE   *pSrcFile = fopen((const char *)(PackFileHead->FileInfoA[i].FileName), "rb+");
        if (pSrcFile==NULL)
        {
            return false;
        }
        chdir( ucPrintInfo );
        FILE   *pDstFile = fopen((const char *)(PackFileHead->FileInfoA[i].FileName), "wb+");
        
        if (pDstFile==NULL)
        {
            fclose(pSrcFile);
            return false; 
        }
        UINT8 * ReadPtr = DstBuff;

        SINT32 CompSize = 0;
        UINT8  *ptr = TemBuff;
        if((CompSize = fread (ptr, 1, COMP_BUF_SIZE, pSrcFile)) > 0)
        {
            SINT32 UnCompSize = inflate(ptr, DstBuff, CompSize);
            if(UnCompSize < 0)
            {
                OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] inflate(%s) error", (UINT32)(const char *)(PackFileHead->FileInfoA[i].FileName));
                return false;
            }

            #define MAX_WRITE_FILE_BLOCK   (1024 * 1000 * 4)
            UINT32 Writesize = 0;

            if(UnCompSize < MAX_WRITE_FILE_BLOCK)
            {
               Writesize = fwrite(ReadPtr, 1, UnCompSize, pDstFile);
               OAM_LOGSTR3(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] write file[%s/%s] size[%d] ", (int)ucPrintInfo, (UINT32)(const char *)(PackFileHead->FileInfoA[i].FileName), Writesize);
            }   
            else
            {
                //开始拷贝
                UINT32 nSize = RW_FILE_BLOCK;
                Writesize = 0;
                UINT32 readsize =0;
                while(UnCompSize > 0)
                {
                    if(nSize > UnCompSize)
                    {
                        nSize = UnCompSize;
                    }
                    readsize = fwrite(ReadPtr, 1, nSize, pDstFile);
                    fflush(pDstFile);
                    ReadPtr    += nSize;
                    UnCompSize -= nSize;
                    Writesize  += readsize;
                    OAM_LOGSTR3(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] write file[%s/%s] size[%d] ", (int)ucPrintInfo, (UINT32)(const char *)(PackFileHead->FileInfoA[i].FileName), Writesize);
                }
            }
		#ifdef WBBU_CODE
	     if(check_rru_file(PackFileHead->FileInfoA[i].FileName)==true)
            {
             //   LOG(LOG_CRITICAL, 0, "write to RamDisk!!!\n");
                char ramddir[60];
		    FILE *fd1;
		   memset(ramddir, 0, 60);
                strcat(ramddir, "/RAMD:0/");
                strcat(ramddir, RAMDISK_DOWNLOAD_PATH);
		   chdir(ramddir);
                //strcat(filename1, PackFileHead->FileInfoA[i].FileName);
                //fd1 = fopen(filename1, O_WRONLY | O_CREAT | O_TRUNC, 0664);
                fd1 = fopen(PackFileHead->FileInfoA[i].FileName, "wb+");
                if ( fd1 == NULL )
                {
                    LOG1(LOG_CRITICAL, 0, "Open file %s on RamDisk error! \n",(int)PackFileHead->FileInfoA[i].FileName);
                    return ERROR;
                }
		  if(UnCompSize < MAX_WRITE_FILE_BLOCK)
                {
                   Writesize = fwrite(ReadPtr, 1, UnCompSize, fd1);
                   OAM_LOGSTR2(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC1] write file[%s] to RAMD, size[%d] ", (UINT32)(const char *)(PackFileHead->FileInfoA[i].FileName), Writesize);
                }   
                else
                {
                    //开始拷贝
                    UINT32 nSize = RW_FILE_BLOCK;
                    Writesize = 0;
                    UINT32 readsize =0;
                    while(UnCompSize > 0)
                    {
                        if(nSize > UnCompSize)
                        {
                            nSize = UnCompSize;
                        }
                        readsize = fwrite(ReadPtr, 1, nSize, fd1);
                        fflush(fd1);
                        ReadPtr    += nSize;
                        UnCompSize -= nSize;
                        Writesize  += readsize;
                        OAM_LOGSTR2(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] write file[%s] to RAMD, size[%d] ", (UINT32)(const char *)(PackFileHead->FileInfoA[i].FileName), Writesize);
                    }
                }
		  fclose(fd1);
            }
		 #endif
        }
        /////////////////////////////////////////////////////
        fclose(pDstFile);
        fclose(pSrcFile);
    }

    chdir( ucPrintInfo );
    copy( "loadVersion.bin", "loadVersion.txt" );
    rm( "loadVersion.bin" );
    SetAttrib( plane, PackFileHead, true );
	WriteToNvRam();
    return true;
}

void CTaskFfpClient :: WriteToNvRam()
{
	UINT8 ucVer[16];
    UINT32 ulVerA=0, ulVerB=0;
    STATUS status = chdir( "/ata0a/btsA" );
        FILE* pfile;
    if( OK == status )
    {
        pfile = fopen("loadVersion.txt", "r");
        if( NULL != pfile )
        {
            fread( ucVer, 1, 16, pfile );
            ulVerA = inet_addr((char*)ucVer);
            fclose(pfile);
            pfile = NULL;
        }
    }
    status = chdir( "/ata0a/btsB" );
    pfile = fopen("loadVersion.txt", "r");
    if( OK == status )
    {
        if( NULL != pfile )
        {
            fread( ucVer, 1, 16, pfile );
            ulVerB = inet_addr((char*)ucVer);
            fclose(pfile);
            pfile = NULL;
        }
    }

    T_BootLoadState bootState;
    bspNvRamRead((char*)&bootState, (char *)NVRAM_BASE_ADDR_BOOT_STATE, sizeof(bootState));
    bootState.LoadVersion_A = ulVerA;
    bootState.LoadVersion_B = ulVerB;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_BOOT_STATE, (char *)&bootState, sizeof(bootState));

    OAM_LOGSTR2(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Write to NvRam A[0x%08X] B[0x%08X] ", ulVerA, ulVerB );
}
void CTaskFfpClient :: InflateTest()
{
    FILE *pFile = fopen("D:\\filetest\\ata0a\\temp\\BTSB.1.1.1.1.bin", "rb");
    if( NULL == pFile )
    	return;
    if(true != SM_UnPackInflateFile(pFile))              //解包同时将文件解压缩
    {
        OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] sm_unpackinflatefile() error");
    }
}

void CTaskFfpClient::UploadCalibrationFile(CMessage &rMsg)
{
    UINT8 *filename = (UINT8*)rMsg.GetDataPtr();
    UINT8 arrFullPathFileName[200] = {0};
    getFullPathFileName(arrFullPathFileName, filename);    
    if (true == FTPCalibrationFile(filename, arrFullPathFileName))
        {
        //通知EMS，FTP结束，并告知文件名
        OAM_LOGSTR1(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC]FTP calibration file[%s] SUCCESS.", (int)filename);
        UINT32 ulDataLen = strlen((char*)filename) + 1;
        CComMessage *pComMsg = new(this, ulDataLen + 2)CComMessage; //+2:trans_id
        if (NULL != pComMsg)
            {
            pComMsg->SetDstTid(M_TID_EMSAGENTTX);
            pComMsg->SetMessageId( M_BTS_EMS_CALIBRATION_FTP_NOTIFY );
            pComMsg->SetSrcTid( this->GetEntityId());
            UINT8 *pDataPtr = (UINT8*)pComMsg->GetDataPtr();
            *(UINT16*)pDataPtr = OAM_DEFAUIT_TRANSID;
            pDataPtr += sizeof(UINT16);
            memcpy( pDataPtr, filename, ulDataLen );
            if(false == CComEntity::PostEntityMessage(pComMsg))
                {
                pComMsg->Destroy();
                }
            else
                OAM_LOGSTR1(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC]FTP calibration file finish. and notify EMS filename[%s].", (int)filename);
            }

        }
    else
        {
        OAM_LOGSTR1(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC]FTP calibration file[%s] FAIL.", (int)filename);
        }

    //delete file
    if (ERROR == remove((const char*)arrFullPathFileName))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC]WARNING!!!Delete calibration file %s failed...", (int)arrFullPathFileName);
        }
    //启动定时器保护ftp任务jy081121
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
    taskSM->setFtpUsingFlag(M_TID_CM, FALSE);
}

#define  M_CAL_UPLOAD_DIR           "system/calibration/"
#include "L3_BTS_PM.h"
extern T_BTSPM_FTPINFO g_tFTPInfo;
bool CTaskFfpClient::FTPCalibrationFile(UINT8 *pFileName, UINT8 *pFullPathFileName)
{
    if ((NULL == pFileName) || (NULL == pFullPathFileName))
        {
        OAM_LOGSTR1( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] WARNING!!!FTPCalibrationFile(%d,),parameter err.", (int)pFileName );
        return false;
        }   
    if ((0 == g_tFTPInfo.uIPAddr)||(0xFFFFFFFF == g_tFTPInfo.uIPAddr))
        {
        OAM_LOGSTR( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] No FTP configure, please config FTP information from EMS!");
        return false;
        }

    UINT32 ctrlSock;
    UINT32 dataSock;
    UINT32 nBytes;
    UINT8  buf[512];
    //STATUS    status;

    struct in_addr FTPServer;
    FTPServer.s_addr = g_tFTPInfo.uIPAddr;
    SINT8  strIpAddr[INET_ADDR_LEN] = {0};
    inet_ntoa_b(FTPServer, strIpAddr);

    FILE *pFile = fopen((const char*)pFullPathFileName, "rb");
    if(NULL == pFile)
        {
        OAM_LOGSTR1( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] WARNING!!!fopen file[%s] error.", (int)pFileName );
        return false;
        }

    OAM_LOGSTR2( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Uploading calibration file[%s] to FTP server[0x%X].", (int)pFileName, g_tFTPInfo.uIPAddr );
    if( ERROR == ftpXfer( (char*)strIpAddr, 
                          g_tFTPInfo.chUserName, 
                          g_tFTPInfo.chPassWord, 
                          NULL, 
                          "STOR %s", 
                          M_CAL_UPLOAD_DIR, 
                          (char*)pFileName, 
                          (int*)&ctrlSock, 
                          (int*)&dataSock) )
        {
        OAM_LOGSTR2( LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Upload calibration file[%s] to FTP server[0x%x] ERROR.", (int)pFileName, g_tFTPInfo.uIPAddr );
        fclose(pFile);
        return false;
        }

    memset(buf, 0, sizeof(buf));
    int totalBytes = 0;
    while((nBytes = fread(buf, 1, 512, pFile)) > 0 )
        {
        int num = 0;
        num = ::write(dataSock, (char*)buf, nBytes );
        if ((ERROR == num) || (num != nBytes))
            {
            OAM_LOGSTR(LOG_SEVERE, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Encounter error when uploading calibration file. Please check network.");
            return false;
            }
        totalBytes += num;
        memset(buf, 0, sizeof(buf));
        }
    OAM_LOGSTR1(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] Uploading calibration file[%d Bytes] finish.", totalBytes);

    fclose(pFile);
    close(dataSock);

    if (ftpReplyGet(ctrlSock, TRUE) != FTP_COMPLETE)
        {
        //status = false;
        }
    if (ftpCommand(ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
        {
        //status = false;
        }
    close(ctrlSock);
    return true;
}


#include "l3BootTask.h"
UINT8* CTaskFfpClient::getFullPathFileName(UINT8 *pFullName, UINT8 *filename)
{
    sprintf((char*)pFullName, "%s%s", RAMDISK_CALIBRATION_DIR, filename);
    return (pFullName);
}


//*
bool  CTaskFfpClient:: SetAttrib( SINT32 plane, const TPackFileHead *PackFileHead, bool bReadOnly )
{
	char ucFlag[4], ucPrintInfo[40];
	strcpy( ucFlag, (bReadOnly)?"+RS":"-RS" );
	strcpy( ucPrintInfo, (plane==0) ? "/ata0a/btsA" : "/ata0a/btsB" );
	chdir( ucPrintInfo );
//	STATUS status;
	for( UINT8 uc=0; uc<PackFileHead->FileCnt; uc++ )
	{
		memset( ucPrintInfo, 0, 40 );
		strcpy( ucPrintInfo, PackFileHead->FileInfoA[uc].FileName );
        //UINT8 ucc = strcmp (ucPrintInfo, "loadVersion.bin");
		if(strcmp (ucPrintInfo, "loadVersion.bin") == 0)
		    strcpy( ucPrintInfo, "loadVersion.txt" );
      	if( OK != attrib( ucPrintInfo, ucFlag ) )
      	{
		    strcat( ucPrintInfo, "   ---   ");
		    strcat( ucPrintInfo, ucFlag );
			strcat( ucPrintInfo, "   Fail");
			OAM_LOGSTR(LOG_DEBUG3, L3SM_ERROR_REV_MSG, ucPrintInfo);
		}
	}
    OAM_LOGSTR(LOG_DEBUG3, L3SM_ERROR_PRINT_SM_INFO, "[tFtpC] SetAttrib() Success" );
    return true;
}
//*/

