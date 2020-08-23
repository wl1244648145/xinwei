/*----------------------------------------------------------------------------------
    telnet.c - 命令行模块TELNET

版权所有 2004 -2006 信威公司深研所BSC项目组.

修改历史记录
--------------------

04.00.01A,  08-20-2004,     L.Y     创建
------------------------------------------------------------------------------------*/

/*
模块功能
该模块为处理标准TELNET 输入输出
...
INCLUDE FILES: cliShell.h
*/

/* 
* 头文件 
*/
#include "sysos.h"
#include "msg.h"

#include "cliTelnet.h"
#include "cliShell.h"

#define THIS_FILE_ID FILE_CLITELNET_C

#ifndef __USE_OTHER_TELNETD__

#ifndef RL_STATIC
#define RL_STATIC static
#endif

#define DOUBLE_BYTE(low, high)  (low + (high << 8))
#define __INVERT_RFC__

/* 
* 全局变量 
*/

extern _INT SendMsgTelnet(LONG_MESSAGE *pstMsg);

/* 
* 本地变量 
*/
//#define     __CLI_DEBUG_TELNET__
#ifdef __CLI_DEBUG_TELNET__
static FILE *telnetDebug;
#endif /* __CLI_DEBUG_TELNET__ */

static OPT_INIT requiredOptions[] =
{
	
    { kCLI_TELOPT_ECHO       ,  kCLI_TC_WILL, TRUE,   FALSE},
    { kCLI_TELOPT_SGA        ,  0,            TRUE,   FALSE},
    { kCLI_TELOPT_TTYPE      ,  kCLI_TC_DO,   TRUE,   FALSE},
    { kCLI_TELOPT_NAWS       ,  kCLI_TC_DO,   TRUE,   FALSE},
    { kCLI_TELOPT_LFLOW      ,  0,            FALSE,  TRUE},
    { kCLI_TELOPT_LINEMODE   ,  kCLI_TC_DO, TRUE,  FALSE},
    { kCLI_TELOPT_STATUS     ,  0,            TRUE,   FALSE}
    
};

const static DBT_INFO stateInfo[] =
{
    {kCLI_TS_INVALID , "Bad "},
    {kCLI_TS_DATA    , "Data"},
    {kCLI_TS_IAC     , "Cmd "},
    {kCLI_TS_CR      , "CrLf"},
    {kCLI_TS_SB      , "Sub "},
    {kCLI_TS_WILL    , "Will"},
    {kCLI_TS_WONT    , "Wont"},
    {kCLI_TS_DO      , "Do  "},
    {kCLI_TS_DONT    , "Dont"},
    {kCLI_TS_CURSOR  , "Curs"},
    {kCLI_TS_ESC     , "Esc "}
};

const static DBT_INFO dbtiCommand[] =
{
    {kCLI_TC_IAC     , "interpret as command: "},
    {kCLI_TC_DONT    , "you are not to use option "},
    {kCLI_TC_DO      , "please, you use option "},
    {kCLI_TC_WONT    , "I won't use option "},
    {kCLI_TC_WILL    , "I will use option "},
    {kCLI_TC_SB      , "interpret as subnegotiation "},
    {kCLI_TC_GA      , "you may reverse the line "},
    {kCLI_TC_EL      , "erase the current line "},
    {kCLI_TC_EC      , "erase the current character "},
    {kCLI_TC_AYT     , "are you there "},
    {kCLI_TC_AO      , "abort output--but let prog finish "},
    {kCLI_TC_IP      , "interrupt process--permanently "},
    {kCLI_TC_BREAK   , "break "},
    {kCLI_TC_DM      , "data mark--for connect. cleaning "},
    {kCLI_TC_NOP     , "nop "},
    {kCLI_TC_SE      , "end sub negotiation "},
    {kCLI_TC_EOR     , "end of record (transparent mode) "},
    {kCLI_TC_ABORT   , "Abort process "},
    {kCLI_TC_SUSP    , "Suspend process "},
    {kCLI_TC_EOF     , "End of file "},
    {kCLI_TC_SYNCH   , "for telfunc calls "}
};

const static DBT_INFO dbtiOption[] =
{
    {kCLI_TELOPT_BINARY          ,  "8-bit data path"},
    {kCLI_TELOPT_ECHO            ,  "echo"},
    {kCLI_TELOPT_RCP             ,  "prepare to reconnect"},
    {kCLI_TELOPT_SGA             ,  "suppress go ahead"},
    {kCLI_TELOPT_NAMS            ,  "approximate message size"},
    {kCLI_TELOPT_STATUS          ,  "give status"},
    {kCLI_TELOPT_TM              ,  "timing mark"},
    {kCLI_TELOPT_RCTE            ,  "remote controlled transmission and echo"},
    {kCLI_TELOPT_NAOL            ,  "negotiate output line width"},
    {kCLI_TELOPT_NAOP            ,  "negotiate output page size"},
    {kCLI_TELOPT_NAOCRD          ,  "negotiate CR disposition"},
    {kCLI_TELOPT_NAOHTS          ,  "negotiate horizontal tabstops"},
    {kCLI_TELOPT_NAOHTD          ,  "negotiate horizontal tab disposition"},
    {kCLI_TELOPT_NAOFFD          ,  "negotiate formfeed disposition"},
    {kCLI_TELOPT_NAOVTS          ,  "negotiate vertical tab stops"},
    {kCLI_TELOPT_NAOVTD          ,  "negotiate vertical tab disposition"},
    {kCLI_TELOPT_NAOLFD          ,  "negotiate output LF disposition"},
    {kCLI_TELOPT_XASCII          ,  "extended ascic character set"},
    {kCLI_TELOPT_LOGOUT          ,  "force logout"},
    {kCLI_TELOPT_BM              ,  "byte macro"},
    {kCLI_TELOPT_DET             ,  "data entry terminal"},
    {kCLI_TELOPT_SUPDUP          ,  "supdup protocol"},
    {kCLI_TELOPT_SUPDUPOUTPUT    ,  "supdup output"},
    {kCLI_TELOPT_SNDLOC          ,  "send location"},
    {kCLI_TELOPT_TTYPE           ,  "terminal type"},
    {kCLI_TELOPT_EOR             ,  "end or record"},
    {kCLI_TELOPT_TUID            ,  "TACACS user identification"},
    {kCLI_TELOPT_OUTMRK          ,  "output marking"},
    {kCLI_TELOPT_TTYLOC          ,  "terminal location number"},
    {kCLI_TELOPT_3270REGIME      ,  "3270 regime"},
    {kCLI_TELOPT_X3PAD           ,  "X.3 PAD"},
    {kCLI_TELOPT_NAWS            ,  "window size"},
    {kCLI_TELOPT_TSPEED          ,  "terminal speed"},
    {kCLI_TELOPT_LFLOW           ,  "remote flow control"},
    {kCLI_TELOPT_LINEMODE        ,  "Linemode option"},
    {kCLI_TELOPT_XDISPLOC        ,  "X Display Location"},
    {kCLI_TELOPT_OLD_ENVIRON     ,  "Environment variables (Old)"},
    {kCLI_TELOPT_AUTHENTICATION  ,  "Authenticate"},
    {kCLI_TELOPT_ENCRYPT         ,  "Encryption option"},
    {kCLI_TELOPT_NEW_ENVIRON     ,  "Environment variables (New)"},
    {kCLI_TELOPT_EXOPL           ,  "extended-options-CLI_LIST"}
};


static DBT_DES dbtdOption =
{
    (DBT_INFO *) &dbtiOption,
    "Option",
    ARRAY_SIZE(dbtiOption)
};


static struct TelStateInfo
{
    TEL_STATE  state;
    _CHAR    *pName;
} TelStateInfo[] =
{
    {TS_Invalid,    "Invalid"   },
    {TS_No,         "No"        },
    {TS_WantNo,     "Want No"   },
    {TS_WantYes,    "Want Yes"  },
    {TS_Yes,        "Yes"       }
};


/*-----------------------------------------------------------------------*/

RL_STATIC _CHAR * TELNET_OptionString(_UCHAR ucOption)
{
    _SHORT       index;
    DBT_DES *pDbtd = &dbtdOption;
    DBT_INFO     *pDbti = pDbtd->info;

    for (index = 0; index < pDbtd->count; index++)
    {
        if (pDbti->option == ucOption)
        {
            return pDbti->description;
        }
        pDbti++;
    }
    return NULL;
}


/*-----------------------------------------------------------------------*/

RL_STATIC PAIR_STATE * TELNET_NewOption(CLI_ENV *pCliEnv, _UCHAR option, _CHAR desired )
{
    PAIR_STATE   *pOption     = MMISC_GetOptHandled(pCliEnv);
    TEL_STATE     hostState   = TS_Invalid;
    TEL_STATE     clientState = TS_Invalid;
    _INT       index;

    for (index = 0; index < kCLI_MAX_OPT_HANDLED; index++, pOption++)
    {
        if (pOption->option == option)
            return pOption;

        if (pOption->option == 0)
        {
            pOption->option             = option;
            pOption->desired            = desired;
            pOption->client.count       = 0;
            pOption->client.name        = 'C';
            pOption->client.optState    = clientState;
            pOption->client.queueState  = QUEUE_Empty;
            pOption->host.count         = 0;
            pOption->host.name          = 'H';
            pOption->host.optState      = hostState;
            pOption->host.queueState    = QUEUE_Empty;

            return pOption;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/

RL_STATIC PAIR_STATE * TELNET_GetOption(CLI_ENV *pCliEnv, _UCHAR option)
{
    _INT       index;
    PAIR_STATE *pOption = MMISC_GetOptHandled(pCliEnv);

    /* find existing option */
    for (index = 0; index < kCLI_MAX_OPT_HANDLED; index++, pOption++)
    {
        if (pOption->option == option)
            return pOption;
    }

    /* create default new option */
    pOption = TELNET_NewOption(pCliEnv, option, FALSE);

    return pOption;
}

/*-----------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
static _CHAR log_start = FALSE;
static _CHAR   ErrorMsg[64];
#endif /* __CLI_DEBUG_TELNET__ */

#ifdef __CLI_DEBUG_TELNET__

#define kFORMAT_TELNET_SHOW "%-6s  %-4s  %-8s  %-8s  %-16s  %s\r\n"

RL_STATIC _VOID TELNET_Show(CLI_ENV *pCliEnv, _CHAR from, _UCHAR option, _UCHAR action, _CHAR *pMsg)
{
    PAIR_STATE *pOption  = TELNET_GetOption(pCliEnv, option);
    _CHAR     *pSide    = ('H' == from ? "Host  " : "Client");
    _CHAR     *pInfo    = TELNET_OptionString(option);
    _CHAR     *pAction  = " - ";
    _CHAR     *pHost    = TelStateInfo[pOption->host.optState].pName;
    _CHAR     *pClient  = TelStateInfo[pOption->client.optState].pName;
    _CHAR     buffer[256];

    if (0 == action)
        return;

    ErrorMsg[0] = 0;

    switch (action)
    {
        case kCLI_TC_WILL:
            pAction = "WILL";
            break;
        case kCLI_TC_WONT:
            pAction = "WONT";
            break;
        case kCLI_TC_DO:
            pAction = "DO";
            break;
        case kCLI_TC_DONT:
            pAction = "DONT";
            break;
        default:
            pAction = "HUH?";
            break;
    }

    if (FALSE == log_start)
    {
        sprintf(buffer, kFORMAT_TELNET_SHOW,
                "Side", "Req.", "Host", "Client", "Option", "Message");
        LogWrite(telnetDebug, buffer);
        sprintf(buffer, kFORMAT_TELNET_SHOW,
                "----", "----", "----", "------", "------", "-------");
        LogWrite(telnetDebug, buffer);
        log_start = TRUE;
    }

    sprintf(buffer, kFORMAT_TELNET_SHOW,
            pSide, pAction, pHost, pClient, pInfo, ErrorMsg);
    LogWrite(telnetDebug, buffer);

    CLI_EXT_WriteStr(pCliEnv, buffer);
}
#else
#define TELNET_Show(pCliEnv, from, option, action, pMsg)
#endif /* __CLI_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
RL_STATIC _VOID TELNET_Error(CLI_ENV *pCliEnv, _CHAR *pMsg)
{
    strncpy(ErrorMsg, pMsg, sizeof(ErrorMsg));
}
#else
#define TELNET_Error(pChannel, pMsg)
#endif /* __CLI_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
RL_STATIC _VOID TELNET_Log(_CHAR from, _UCHAR option, _UCHAR action)
{
    _CHAR  buffer[256];
    _CHAR *fromText   = 'H' == from ? "Host" : "Term";
    _CHAR *optionText = TELNET_OptionString(option);
    _CHAR *actionText;

    switch (action)
    {
        case kCLI_TC_WILL:
            actionText = "WILL";
            break;
        case kCLI_TC_WONT:
            actionText = "WONT";
            break;
        case kCLI_TC_DO:
            actionText = "DO";
            break;
        case kCLI_TC_DONT:
            actionText = "DONT";
            break;
        default:
            actionText = " ? ";
            break;
    }

    sprintf(buffer, "%-4s %-4s %s\r\n",
            fromText, actionText, optionText);
    printf("\n%s",buffer);
    LogWrite(telnetDebug, buffer);
}
#else
#define TELNET_Log(from, option, action)
#endif /* __CLI_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

RL_STATIC _CHAR * TELNET_StateInfo(TEL_STATE state)
{
    if ((state < 0) || (state > ARRAY_SIZE(TelStateInfo)))
        state = TS_Invalid;

    return TelStateInfo[state].pName;
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID TELNET_StartOption(CLI_ENV *pCliEnv, _UCHAR data)
{
    _CHAR  *pBuffer    = MCONN_OptBufferPtr(pCliEnv);
    _INT     index     = MCONN_GetOptBufferIndex(pCliEnv);
    
#ifdef __CLI_DEBUG_TELNET__
    _UCHAR    subOption = MCONN_GetSubOption(pCliEnv);
    _CHAR     *info      = TELNET_OptionString(subOption);
#endif /* __CLI_DEBUG_TELNET__ */

    /* beginning capture */
    if (0 > index)
    {
#ifdef __CLI_DEBUG_TELNET__
        info     = TELNET_OptionString(data);
        CLI_EXT_WriteStr(pCliEnv, "Start option data: ");
        CLI_EXT_WriteStrLine(pCliEnv, info);
#endif

        MCONN_SetSubOption(pCliEnv, data);
        MCONN_SetOptBufferIndex(pCliEnv, 0);
        return;
    }

    pBuffer[index] = (_CHAR) data;

    /* prevent pBuffer overrun */
    if ( ++index >= kCLI_OPT_BUF_SIZE)
        index = kCLI_OPT_BUF_SIZE;

    MCONN_SetOptBufferIndex(pCliEnv, index);
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID TELNET_OptWindowSize(CLI_ENV *pCliEnv)
{
    _UCHAR      *buffer = (_UCHAR *) MCONN_OptBufferPtr(pCliEnv);
    _SHORT    width;
    _SHORT    height;

    width  = DOUBLE_BYTE(buffer[1], buffer[0]);
    height = DOUBLE_BYTE(buffer[3], buffer[2]);

    MSCRN_SetWidth(pCliEnv, width);
    MSCRN_SetHeight(pCliEnv, height);
}

/* 显示当前设置 */
_VOID CLI_TELNET_Status(CLI_ENV *pCliEnv)
{
    PAIR_STATE   *pOptions  = MMISC_GetOptHandled(pCliEnv);
    _INT        index;
    _CHAR       *pHost;
    _CHAR       *pClient;
    _CHAR       *pInfo     = NULL;
    _CHAR       *pWant;
    _CHAR        buffer[128];

#define STATUS_FORMAT   "%-30s   %-4s  %-8s   %-8s"

    CLI_EXT_WriteStrLine(pCliEnv, "");
    sprintf(buffer, STATUS_FORMAT, "Option", "Want", "Host", "Client");
    CLI_EXT_WriteStrLine(pCliEnv, buffer);

    if (NULL == pOptions)
        return;

    for (index = 0; index < kCLI_MAX_OPT_HANDLED; index++, pOptions++)
    {
        if (0 == pOptions->option)
            continue;

        pInfo   = TELNET_OptionString(pOptions->option);
        pHost   = TELNET_StateInfo(pOptions->host.optState);
        pClient = TELNET_StateInfo(pOptions->client.optState);
        pWant   = pOptions->desired ? "Yes" : "No";

        sprintf(buffer, STATUS_FORMAT, pInfo, pWant, pHost, pClient);
        CLI_EXT_WriteStrLine(pCliEnv, buffer);
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC _UCHAR TELNET_Enable(CLI_ENV *pCliEnv, _UCHAR option, _CHAR enable)
{
    _UCHAR    reply     = 0;
    PAIR_STATE   *pOption   = TELNET_GetOption(pCliEnv, option);
    OPTION_STATE *pStatus   = &pOption->client;

    /* keep track of whether we want this option */
    pOption->desired = enable;
    
/*

      If we decide to ask them to enable:
         NO            them=WANTYES, send DO.
         YES           Error: Already enabled.
         WANTNO  EMPTY If we are queueing requests, themq=OPPOSITE;
                       otherwise, Error: Cannot initiate new request
                       in the middle of negotiation.
              OPPOSITE Error: Already queued an enable request.
         WANTYES EMPTY Error: Already negotiating for enable.
              OPPOSITE themq=EMPTY.
*/

    if (enable)
    {
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            pStatus->optState = TS_WantYes;
#ifdef __INVERT_RFC__
            reply = kCLI_TC_WILL;
#else
            reply = kCLI_TC_DO; 
#endif
            break;
        case TS_Yes:
            TELNET_Error(pCliEnv, "Already enabled"); 
            break;
        case TS_WantNo:
            TELNET_Error(pCliEnv, "Waiting for NO"); 
            break;
        case TS_WantYes:
            if (QUEUE_Opposite == pStatus->queueState)
                pStatus->queueState = QUEUE_Empty;
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state"); 
            break;
        }
    }
    else
    {
/*
      If we decide to ask them to disable:
         NO            Error: Already disabled.
         YES           them=WANTNO, send DONT.
         WANTNO  EMPTY Error: Already negotiating for disable.
              OPPOSITE themq=EMPTY.
         WANTYES EMPTY If we are queueing requests, themq=OPPOSITE;
                       otherwise, Error: Cannot initiate new request
                       in the middle of negotiation.
              OPPOSITE Error: Already queued a disable request.
*/
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            TELNET_Error(pCliEnv, "Already disabled");
            break;
        case TS_Yes:
            pStatus->optState = TS_WantNo;
#ifdef __INVERT_RFC__
            reply = kCLI_TC_WONT;
#else
            reply = kCLI_TC_DONT; 
#endif
            break;
        case TS_WantNo:
            if (QUEUE_Opposite == pStatus->queueState)
                pStatus->queueState = QUEUE_Empty;
            else
                TELNET_Error(pCliEnv, "Queue is empty!"); 
            break;
        case TS_WantYes:
            TELNET_Error(pCliEnv, "Unexpected DONT");
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state"); 
            break;
        }
    }

    TELNET_Show(pCliEnv, 'H', option, reply, "");

    return reply;
}


/* tell client all the options we want */
RL_STATIC _INT TELNET_Required(CLI_ENV *pCliEnv)
{
    PAIR_STATE    *pOption;
    _CHAR        desired;
    _UCHAR      action;
    _UCHAR      option;
    _INT           index;
    OPT_INIT       *pInit = requiredOptions;

#ifdef __CLI_DEBUG_TELNET__
    _CHAR       *pInfo;
#endif

    for (index = 0; index < ARRAY_SIZE(requiredOptions); index++, pInit++)
    {
        option  = pInit->option;
        desired = pInit->desired;

        pOption = TELNET_NewOption(pCliEnv, option, desired);
        if (NULL == pOption)
            return CLI_ERROR;

#ifdef __CLI_DEBUG_TELNET__
        pInfo  = TELNET_OptionString(option);
#endif

        if (0 < pInit->action)
            CLI_TELNET_Handshake(pCliEnv, pInit->option, pInit->action);

        if (pInit->desired)
        {
            action = TELNET_Enable(pCliEnv, option, desired);
            if (0 != action)
                CLI_TELNET_Handshake(pCliEnv, option, action);
        }
    }
    return SUCC;
}

/*-----------------------------------------------------------------------*/

_INT CLI_TELNET_Init(CLI_ENV *pCliEnv)
{
    COM_CHAN  *pChannel = MMISC_GetChannel(pCliEnv);
    _INT     status;
    _INT       optSize  = sizeof(PAIR_STATE) * kCLI_MAX_OPT_HANDLED;

    if (SUCC != (status = CLI_UTIL_Init(pCliEnv)))
        return status;

    pChannel->ThreadState    = kThreadWorking;

    MCONN_SetSubOption(pCliEnv,      0);
    MCONN_SetConnType(pCliEnv,       kTELNET_CONNECTION);
    MCONN_SetRecvState(pCliEnv,      kCLI_TS_DATA);
    MCONN_SetOptBufferIndex(pCliEnv, -1);

    MSCRN_SetWidth(pCliEnv,          kCLI_DEFAULT_WIDTH);
    MSCRN_SetHeight(pCliEnv,         kCLI_DEFAULT_HEIGHT);

    memset(MMISC_GetOptHandled(pCliEnv), 0, optSize);

    //CLI_TELNETD_AddSession(pCliEnv);

    /* tell client options we want */
    TELNET_Required(pCliEnv);

    return SUCC;
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID TELNET_SaveTerminalType(CLI_ENV *pCliEnv)
{
    _CHAR  *pBuffer  = MCONN_OptBufferPtr(pCliEnv);
    _CHAR  *pName    = MCONN_TermType(pCliEnv);
    _INT  index    = MCONN_GetOptBufferIndex(pCliEnv);

    pBuffer++;
    strncpy(pName, pBuffer, CLI_MIN(index, kCLI_TERM_TYPE_SIZE));
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID TELNET_SaveOption(CLI_ENV *pCliEnv)
{
    _CHAR      saved     = TRUE;
    _UCHAR      subOption = MCONN_GetSubOption(pCliEnv);
    
#ifdef __CLI_DEBUG_TELNET__
    _CHAR       *info      = TELNET_OptionString(subOption);
#endif

    switch (subOption)
    {
        case kCLI_TELOPT_NAWS:
            TELNET_OptWindowSize(pCliEnv);
            break;
        case kCLI_TELOPT_TTYPE:
            TELNET_SaveTerminalType(pCliEnv);
            break;
        default:
            saved = FALSE;
            break;
    }

#ifdef __CLI_DEBUG_TELNET__
    if (saved)
        CLI_EXT_WriteStr(pCliEnv, "Saved:  ");
    else
        CLI_EXT_WriteStr(pCliEnv, "Tossed: ");
    CLI_EXT_WriteStrLine(pCliEnv, info);
#endif

    MCONN_SetOptBufferIndex(pCliEnv, -1);
    MCONN_SetRecvState(pCliEnv, kCLI_TS_DATA);
    MCONN_SetSubOption(pCliEnv, 0);
}

/*-----------------------------------------------------------------------*/

 _INT CLI_TELNET_Send(CLI_ENV *pEnv, _CHAR *pBuf, _INT nBufLen)
{

    //send to platform to write 
    LONG_MESSAGE Msg;

    Msg.stHeader.ucDstTaskNo = IP_TASK_ID;
    Msg.stHeader.ucSrcTaskNo = CLI_TASK_ID;
    Msg.stHeader.usMsgId        = MSG_IP_DATAREQ;
    Msg.stHeader.usMsgLen      = nBufLen; 

    //Msg.ucBuffer[0] = MMISC_GetFsmId(pEnv) + (IP_FSM_CLI_FIRST_CLIENT-1) ;  //加上偏移量;
    SYS_MEMCPY(Msg.ucBuffer, pBuf, nBufLen);
    //printf("\n\rTelnet send msg:");
    SendMsgTelnet(&Msg);
    return SUCC;
    
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID TELNET_Request(CLI_ENV *pCliEnv, _UCHAR option)
{
    COM_CHAN *pChannel  = MMISC_GetChannel(pCliEnv);
    _UCHAR       command[] = {kCLI_TC_IAC, kCLI_TC_SB, 0, 
                             1, kCLI_TC_IAC, kCLI_TC_SE};

    switch (option)
    {
        case kCLI_TELOPT_STATUS:
        case kCLI_TELOPT_TTYPE:
            break;
        default:
            return;
    }

    command[2] = option;
    CLI_TELNET_Send(pCliEnv, (_CHAR *) command, sizeof(command));
}

/*-----------------------------------------------------------------------*/

RL_STATIC _VOID TELNET_Negotiate(CLI_ENV *pCliEnv, _UCHAR option, _UCHAR action)
{
    PAIR_STATE   *pOption  = NULL;
    _UCHAR      reply    = 0;

#ifdef __CLI_DEBUG_TELNET__
    TELNET_Log('C', option, action); 
#endif /* __CLI_DEBUG_TELNET__ */

    if (NULL == (pOption = TELNET_GetOption(pCliEnv, option)))
        return;

    reply = CLI_TELNET_StateChange(pCliEnv, 'C', option, action);

    CLI_TELNET_Handshake(pCliEnv, option, reply);
}

/*-----------------------------------------------------------------------*/

_INT CLI_TELNET_Recv(CLI_ENV *pCliEnv, _UCHAR charIn, _CHAR *pBuf, _INT *bytesReturned)
{
    _UCHAR    state  = MCONN_GetRecvState(pCliEnv);
    _INT       status  = SUCC;

    *bytesReturned = 0;

    switch (state) 
    {           
        case kCLI_TS_DATA:
            if (kCLI_TC_IAC == charIn)
                state = kCLI_TS_IAC;
            else
                pBuf[(*bytesReturned)++] = charIn;
            break;
        case kCLI_TS_IAC:
            switch (charIn) {
                case kCLI_TC_IP:
                case kCLI_TC_BREAK:
                case kCLI_TC_AYT:
                case kCLI_TC_AO:
                case kCLI_TC_EC:
                case kCLI_TC_EL:
                case kCLI_TC_DM:
                case kCLI_TC_EOR:
                case kCLI_TC_EOF:
                case kCLI_TC_SUSP:
                case kCLI_TC_ABORT:
                    break;
                case kCLI_TC_SB:
                    state = kCLI_TS_SB;
                    break;
                case kCLI_TC_WILL:
                    state = kCLI_TS_WILL;
                    break;
                case kCLI_TC_WONT:
                    state = kCLI_TS_WONT;
                    break;
                case kCLI_TC_DO:
                    state = kCLI_TS_DO;
                    break;
                case kCLI_TC_DONT:
                    state = kCLI_TS_DONT;
                    break;
                case kCLI_TC_IAC:
                    pBuf[(*bytesReturned)++] = charIn;
                    state = kCLI_TS_DATA;
                    break;
                case kCLI_TC_SE:
                    TELNET_SaveOption(pCliEnv);
                    state = kCLI_TS_DATA;
                    break;
                default:
                    state = kCLI_TS_DATA;
                    break;
                } /* switch(charIn) */
            break;
        case kCLI_TS_SB:
            if (kCLI_TC_IAC == charIn)
                state = kCLI_TS_IAC;
            else
                TELNET_StartOption(pCliEnv, charIn);
            break;
        case kCLI_TS_WILL:
        case kCLI_TS_WONT:
        case kCLI_TS_DO:
        case kCLI_TS_DONT:
            TELNET_Negotiate(pCliEnv, charIn, state);
            state = kCLI_TS_DATA;
            break;
        default:
#ifndef __CLI_DEBUG_TELNET__
            state = kCLI_TS_DATA;
            break;
#else
            printf("DEFAULT CASE\n");
            status = ERROR_CLI_FAILURE;
#endif
    } /* switch (*state) */
        
    MCONN_SetRecvState(pCliEnv, state);
    return status;
}

/*-----------------------------------------------------------------------*/

_INT CLI_TELNET_Handshake(CLI_ENV *pCliEnv, _UCHAR option, _UCHAR action)
{
    _UCHAR        command[3];

    if (0 == action)
        return SUCC; /* should be an error someday */

    TELNET_Log('H', option, action); 

    command[0] = kCLI_TC_IAC;
    command[1] = action;
    command[2] = option;

    return CLI_TELNET_Send(pCliEnv, (_CHAR *) command, sizeof(command));
}


/*-----------------------------------------------------------------------*/

_UCHAR CLI_TELNET_StateChange(CLI_ENV *pCliEnv, _CHAR from, _UCHAR option, _UCHAR action)
{
    PAIR_STATE   *pOption   = TELNET_GetOption(pCliEnv, option);
    OPTION_STATE *pStatus   = 'H' == from ? &pOption->host : &pOption->client;
    _UCHAR    reply     = 0;
    _UCHAR    affirm    = 0;
    _UCHAR    negate    = 0;

    if (1 == option)
        option = 1;

    if (pStatus->count++ > 4)
    {
        pStatus->count = 0;
    }

    switch (action)
    {
    case kCLI_TS_DONT:
    case kCLI_TS_DO:
        affirm = kCLI_TS_WILL;
        negate = kCLI_TS_WONT;
        break;
    case kCLI_TS_WILL:
    case kCLI_TS_WONT:
        affirm = kCLI_TS_DO;
        negate = kCLI_TS_DONT;
        break;
    }

    switch (action)
    {
    case kCLI_TS_WILL:
        /* send suboption request */
        if ('C' == from)
            TELNET_Request(pCliEnv, option);
    case kCLI_TS_DO:
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            if (pOption->desired)
            {
                reply = affirm;
                pStatus->optState = TS_Yes;
            }
            else
                reply = negate;
            break;
        case TS_Yes:
            break;
        case TS_WantNo:
            if (QUEUE_Empty == pStatus->queueState)
                pStatus->optState   = TS_No;
            else
            {
                pStatus->optState   = TS_Yes;
                pStatus->queueState = QUEUE_Empty;
            } 
            break;
        case TS_WantYes:
            if (QUEUE_Empty == pStatus->queueState)
                pStatus->optState = TS_Yes;
            else
            {
                pStatus->queueState = QUEUE_Empty;
                pStatus->optState = TS_WantNo;
                reply = negate;
            }
            break;
        default:
            TELNET_Error(pCliEnv, "UNKNOWN STATE");
            break;
        }
        break;
        case kCLI_TS_WONT:
        case kCLI_TS_DONT:
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            break;
        case TS_Yes:
            pStatus->optState = TS_No;
            reply = negate;
            break;
        case TS_WantNo:
            if (QUEUE_Empty == pStatus->queueState)
                pStatus->optState = TS_No;
            else
            {
                pStatus->queueState = QUEUE_Empty;
                pStatus->optState = TS_WantYes;
                reply = affirm;
            }
            break;
        case TS_WantYes:
            pStatus->optState = TS_No;
            if (QUEUE_Opposite == pStatus->queueState)
                pStatus->queueState = QUEUE_Empty;
            break;
        default:
            TELNET_Error(pCliEnv, "UNKNOWN STATE");
            break;
        }
        break;
    }

    TELNET_Show(pCliEnv, from, option, action, "");

    return reply;
}

#endif /* __USE_OTHER_TELNETD__ */

