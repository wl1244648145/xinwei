/*-------------------------------------------------------------------
        cliShell.h - 命令行模块命令解释部分

版权所有 2004 -2006 信威公司深研所BSC项目组. 

修改历史记录
--------------------

04.00.01A,  08-05-2004,     L.Y     创建
---------------------------------------------------------------------*/

#ifndef   _CLISHELL_H_
#define _CLISHELL_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */


/*
* 本模块负责用户输入命令的解析
* 
*/


/* 
*   头文件引用 
*/
 

/* 
*   宏定义
*/
#define  MODID   0
#define  kCLI_MAX_LOGIN_LEN		16
#define  kCLI_MAX_PASSWORD_LEN  16
#define  kCLI_MAX_CLI_TASK       2//7


#define     CLI_ERROR                                               0xA000
#define     CLI_STATUS                                              0xA100

#define     ERROR_CLI                     (-10000)
#define     ERROR_CLI_FAILURE             (ERROR_CLI - 1)
#define     ERROR_CLI_RETRY               (ERROR_CLI - 2)
#define       ERROR_CLI_ABORT                        (ERROR_CLI - 3)
#define     ERROR_CLI_TIMEOUT             (ERROR_CLI - 4)
#define     ERROR_CLI_READ                (ERROR_CLI - 5)
#define     ERROR_CLI_WRITE               (ERROR_CLI - 6)
#define     ERROR_CLI_INVALID_PARAM   (ERROR_CLI - 7)
#define       ERROR_CLI_INVALID_VALUE         (ERROR_CLI - 8)
#define       ERROR_CLI_INVALID_NO               (ERROR_CLI - 9)
#define       ERROR_PERM_DENIED                    (ERROR_CLI - 10)
#define       ERROR_CLI_NULL_ENV                    (ERROR_CLI - 11)
#define       ERROR_CLI_NULL_PARAM_LIST     (ERROR_CLI - 12)
#define       ERROR_CLI_LOGIN_FAIL                 (ERROR_CLI - 13)
#define       ERROR_ADD_USER                        (ERROR_CLI - 14)
#define       ERROR_CLI_WRONG_MODE              (ERROR_CLI - 15)
#define       ERROR_CLI_INVALID_SESSION         (ERROR_CLI - 16)
#define       ERROR_CLI_BAD_NODE                    (ERROR_CLI - 17)
#define       ERROR_CLI_AMBIGUOUS_PARAM         (ERROR_CLI - 18)
#define       ERROR_CLI_HANDLER_EXEC_FAILED     (ERROR_CLI - 19)
#define     ERROR_CLI_MISSING_PARAM         (ERROR_CLI - 20)
#define     ERROR_CLI_NO_PARAM_DATA       (ERROR_CLI - 21)
#define     ERROR_CLI_NO_INPUT_DATA             (ERROR_CLI - 22)
#define     ERROR_CLI_INVALID_OPTION            (ERROR_CLI - 23)
#define       ERROR_CLI_NO_ERROR_MSG                (ERROR_CLI - 24)
#define       ERROR_CLI_BAD_COMMAND             (ERROR_CLI - 25)
#define       ERROR_CLI_NO_HANDLERS                 (ERROR_CLI - 26)
#define       ERROR_CLI_INVALID_USER                (ERROR_CLI - 27)
#define       ERROR_CLI_AMBIGUOUS_COMMAND   (ERROR_CLI - 28)
#define       ERROR_CLI_NO_HELP                         (ERROR_CLI - 29)

/* ----------------------------------------------------- */

#define   STATUS_CLI_LOGOUT                         (CLI_STATUS - 1)
#define     STATUS_CLI_EXIT                                 (CLI_STATUS - 2)
#define     STATUS_CLI_EXIT_ALL                         (CLI_STATUS - 3)
#define   STATUS_CLI_INTERNAL_COMMAND   (CLI_STATUS - 4)
#define   STATUS_CLI_NOT_INTERNAL               (CLI_STATUS - 5)
#define     STATUS_CLI_HISTORY_EXEC                 (CLI_STATUS - 6)
#define     STATUS_CLI_HISTORY_EDIT                 (CLI_STATUS - 7)
#define     STATUS_CLI_NO_INTERMEDIATE          (CLI_STATUS - 8)
#define     STATUS_CLI_NO_ERROR                         (CLI_STATUS - 9)
#define     STATUS_CLI_EXIT_TO_ROOT                 (CLI_STATUS - 10)
#define     STATUS_CLI_KILL                                   (CLI_STATUS - 11)
#define     STATUS_CLI_PARAM_NODE                   (CLI_STATUS - 12)

/* ----------------------------------------------------- */

#define SYS_ERROR_SOCKET_GENERAL                -1200
#define SYS_ERROR_SOCKET_CREATE                 ( SYS_ERROR_SOCKET_GENERAL - 1 )
#define SYS_ERROR_SOCKET_BIND                   ( SYS_ERROR_SOCKET_GENERAL - 2 )
#define SYS_ERROR_SOCKET_THREAD                 ( SYS_ERROR_SOCKET_GENERAL - 3 )
#define SYS_ERROR_SOCKET_LISTEN                 ( SYS_ERROR_SOCKET_GENERAL - 4 )
#define SYS_ERROR_SOCKET_ACCEPT                 ( SYS_ERROR_SOCKET_GENERAL - 5 )
#define SYS_ERROR_SOCKET_CREATE_TASK            ( SYS_ERROR_SOCKET_GENERAL - 6 )
#define SYS_ERROR_SOCKET_DELETE                 ( SYS_ERROR_SOCKET_GENERAL - 7 )
#define SYS_ERROR_SOCKET_SHARE                  ( SYS_ERROR_SOCKET_GENERAL - 8 )
#define SYS_ERROR_SOCKET_START                  ( SYS_ERROR_SOCKET_GENERAL - 9 )
#define SYS_ERROR_SOCKET_CONNECT                ( SYS_ERROR_SOCKET_GENERAL - 10 )
#define SYS_ERROR_SOCKET_TIMEOUT                ( SYS_ERROR_SOCKET_GENERAL - 11 )

/* ----------------------------------------------------- */
#define ERROR_MEMMGR_GENERAL                    -500
#define ERROR_MEMMGR_BAD_MEMSIZE                ( ERROR_MEMMGR_GENERAL - 1 )
#define ERROR_MEMMGR_INITIALIZATION             ( ERROR_MEMMGR_GENERAL - 2 )
#define ERROR_MEMMGR_NO_MEMORY                  ( ERROR_MEMMGR_GENERAL - 3 )
#define ERROR_MEMMGR_BAD_POINTER                ( ERROR_MEMMGR_GENERAL - 4 )
#define ERROR_MEMMGR_BAD_FREE                   ( ERROR_MEMMGR_GENERAL - 5 )
#define ERROR_MEMMGR_MEMORY_CORRUPTION          ( ERROR_MEMMGR_GENERAL - 6 )
#define ERROR_MEMMGR_INVALID_LENGTH             ( ERROR_MEMMGR_GENERAL - 7 )

/* ----------------------------------------------------- */

#define ERROR_GENERAL                           -100
#define ERROR_GENERAL_NO_DATA                   ( ERROR_GENERAL - 1  )
#define ERROR_GENERAL_NOT_FOUND                 ( ERROR_GENERAL - 2  )
#define ERROR_GENERAL_ACCESS_DENIED             ( ERROR_GENERAL - 3  )
#define ERROR_GENERAL_NOT_EQUAL                 ( ERROR_GENERAL - 4  )
#define ERROR_GENERAL_ILLEGAL_VALUE             ( ERROR_GENERAL - 5  )
#define ERROR_GENERAL_CREATE_TASK               ( ERROR_GENERAL - 6  )
#define ERROR_GENERAL_NULL_POINTER              ( ERROR_GENERAL - 7  )
#define ERROR_GENERAL_DATA_AMBIG                ( ERROR_GENERAL - 8  )
#define ERROR_GENERAL_FILE_NOT_FOUND            ( ERROR_GENERAL - 9  )
#define ERROR_GENERAL_BUFFER_OVERRUN            ( ERROR_GENERAL - 10 )
#define ERROR_GENERAL_INVALID_RAPIDMARK         ( ERROR_GENERAL - 11 )
#define ERROR_GENERAL_OUT_OF_RANGE              ( ERROR_GENERAL - 12 )
#define ERROR_GENERAL_INVALID_PATH              ( ERROR_GENERAL - 13 )

#define SYS_ERROR_GENERAL                       -1000
#define SYS_ERROR_NO_MEMORY                     ( SYS_ERROR_GENERAL - 1 )
#define SYS_ERROR_MUTEX_CREATE               ( SYS_ERROR_GENERAL - 2)

#define kTELNET_CONNECTION              1
#define kCONSOLE_CONNECTION             2

#ifndef kCLI_TIMEOUT
#define kCLI_TIMEOUT                    1800
#endif
#define kTELNET_TIMEOUT_IN_MINUTES      (kCLI_TIMEOUT/60)

/* number of CliChannel objects in array (# TCP threads) */
#define kTELNET_QUEUE_SIZE 10

//鉴权状态

#define kTELNET_NOT_LOGIN               0
#define kTELNET_USRNAME_LOGIN       1
#define kTELNET_PASSWD_LOGIN        2
#define kTELNET_LOGIN_OK                 3
#define kTELNET_USRNAME_INPUT      4
#define kTELNET_LOGIN_PROCESS       5
#define kTELNET_USR_PROCESS            6


/*-----------------------------------------------------------------------*/

/* DOS console key codes */
#define kCLI_DOSKEY_ESC     ((char)(0xE0))
#define kCLI_DOSKEY_UP      ((char)(0x48))
#define kCLI_DOSKEY_DN      ((char)(0x50))
#define kCLI_DOSKEY_RT      ((char)(0x4D))
#define kCLI_DOSKEY_LT      ((char)(0x4B))
#define kCLI_DOSKEY_PAGE_UP ((char)(0x49))
#define kCLI_DOSKEY_PAGE_DN ((char)(0x51))
#define kCLI_DOSKEY_DEL     ((char)(0x53))

/*----------------------------------------------------------------------*/

#ifndef kCRLF
#define kCRLF       "\x0D\x0A"
#define kCRLFSize   2
#endif

#ifndef kCR
#define kCR     ((char)(0x0d))
#endif

#ifndef kLF
#define kLF     ((char)(0x0a))
#endif

#ifndef kBS
#define kBS     ((char)(0x08))
#endif

#ifndef kDEL
#define kDEL    ((char)(0x7F))
#endif

#ifndef kTAB
#define kTAB    ((char)(0x09))
#endif

#ifndef kHELP
#define kHELP   ((char)(0x3F))
#endif

#ifndef kESC
#define kESC    ((char)(0x1b))
#endif

#ifndef kCHAR_NULL
#define kCHAR_NULL ((char)(0x00))
#endif


/*-----------------------------------------------------------------------*/
/* Future Extensions */

/* various system options */
#define kCLI_DEFAULT_WIDTH                  80      /* default assumed screen width     */
#define kCLI_DEFAULT_HEIGHT                 24      /* default assumed screen height    */
#define kCLI_OPT_BUF_SIZE                       40      /* buffer for telnet suboptions     */
#define kCLI_MAX_PROMPT_TAIL_LEN        8       /* max size of tail-end of prompt   */
#define kCLI_INTERMEDIATE_PROMPT        ">"     /* after line continuation char     */
#define kCLI_DEFAULT_PROMPT_TAIL        "# "    /* always appended to prompt        */
#define kCLI_PROMPT_DELIMITER               "-"     /* separator in intermediate mode   */
#define kCLI_SYSTEM_HELP                        "help"  /* internal system help             */
#define kCLI_HELP_PREFIX                        " "     /* precedes help text               */
#define kCLI_HELP_DELIMITER                 " - "   /* put between cmd and help         */    
#define kCLI_CMD_NO                                 "no"    /* "no" command                     */
#define kCLI_COL_SEPARATOR                  "\t"    
#define kCLI_ROW_SEPARATOR                  "\r\n"  
#define kCLI_PARAM_LEFT_BRACKET          "<"     /* displaying parameters            */
#define kCLI_PARAM_RIGHT_BRACKET        ">"     /* displaying parameters            */
#define kCLI_HELP_WIDTH                         20     /* width of help item area          */
#define kCLI_CHAR_COMMENT                   '#'     /* ignore all text after this       */
#define kCLI_DEFAULT_PATH                   "C:\\"  /* default path for exec'ing files  */
#define kCLI_DEFAULT_LOGIN                  "bts" /* default login for development    */
#define kCLI_DEFAULT_PASSWORD           "12345678" /* default password for development */
#define kCLI_DEFAULT_ACCESS                 "10"    /* default access level for dev.    */
#define kCLI_MOTD                                       "motd"  
#define kCLI_PRINT_CANCEL                       'Q'     /* character to cancel printing     */
#define kCLI_TASK_STACK_SIZE                    8       /* depth of command queueing stack  */
#define kCLI_HELP_KEYS_WIDTH                    45      /* tab stop for edit keys labels    */
#define kCLI_TERM_TYPE_SIZE                     16      /* size for storing terminal name   */
#define kCLI_MAX_OPT_HANDLED                32      /* buffer for telnet options        */
#define kCLI_ERROR_TEXT_SIZE                    64
#define kCLI_MAX_LOG_STRING                     64

#define kCLI_MSG_LOGIN_FAILED       " 登陆失败: "
#define kCLI_MSG_LOGIN_OKAY         " 登陆成功: "
#define kCLI_MORE_TEXT                      "按任意键继续 (Q 退出)"

/* help and error messages */
#define kCLI_MSG_FAIL         "Error: unable to open session"
#define kCLI_MSG_PARAMS_AVAIL "Available parameters:"
#define kCLI_MSG_ERROR_PREFIX "错误: "

#define kCLI_MSG_LOGOUT_IDLE      "\r\n该系统已有%d 分钟无命令输入.\r\n"
#define kCLI_MSG_LOGOUT_REMAINING "空闲 %d 分钟后退出.\r\n"
#define kCLI_MSG_LOGOUT_NOW       "系统退出"

/* error message text */
#define kMSG_ERROR_CLI_INVALID_PARAM      "错误参数"
#define kMSG_ERROR_CLI_AMBIGUOUS_PARAM    "参数过多"
#define kMSG_ERROR_CLI_DEFAULT            "错误格式"
#define kMSG_ERROR_CLI_BAD_COMMAND        "未找到命令,请重新输入."

/* 命令行状态定义 */

#define kCLI_FLAG_LOGON                     0x00000001 /* user is logged onto system        */
#define kCLI_FLAG_ECHO                      0x00000002 /* enables output                    */
#define kCLI_FLAG_MODE                      0x00000004 /* disallow intermediate modes       */
#define kCLI_FLAG_IGNORE                     0x00000008 /* ignore extra parameters           */
#define kCLI_FLAG_FLUSH                     0x00000010 /* need to delete parameter LIST     */
#define kCLI_FLAG_WARN                      0x00000020 /* detailed error descriptions       */
#define kCLI_FLAG_HISTORY                   0x00000040 /* do not clear input line           */
#define kCLI_FLAG_RAPID                         0x00000080 /* interpret rapidmarks as data      */
#define kCLI_FLAG_SNMPAUTO                  0x00000100 /* auto-commit snmp changes          */
#define kCLI_FLAG_HELPCHILD                     0x00000200 /* display child nodes in help       */
#define kCLI_FLAG_HARDWRAP                  0x00000400 /* use hard wrap in output           */
#define kCLI_FLAG_UPDATE                        0x00000800 /* update input buffer for print     */
#define kCLI_FLAG_MYPROMPT                  0x00001000 /* use custom prompt                 */
#define kCLI_FLAG_INPUT                             0x00002000 /* in input mode (for logging)       */
#define kCLI_FLAG_DASHES                         0x00004000 /* uses dashes in error line         */
#define kCLI_FLAG_SNMPATOM                       0x00008000 /* commit write for each snmp var    */
#define kCLI_FLAG_MORE                              0x00010000 /* paginate output                   */
#define kCLI_FLAG_THISHELP                      0x00020000 /* show help for current node        */
#define kCLI_FLAG_PARAMHELP                     0x00040000 /* show help for parameters used     */
#define kCLI_FLAG_LOG_INPUT                     0x00080000 /* enable logging input              */
#define kCLI_FLAG_LOG_IO                        0x00100000 /* enable logging input and output   */
#define kCLI_FLAG_LOG_OUTPUT                0x00200000 /* enable logging output             */

/* 调试选项 */
#define kCLI_FLAG_PARAMS                        0x02000000 /* dump parameter LIST for debugging */
#define kCLI_FLAG_TELNET                            0x04000000 /* dump telnet stream to console     */
#define kCLI_FLAG_STACK                         0x08000000 /* dump parameter LIST to console    */
#define kCLI_FLAG_EXEC                              0x10000000 /* dump exec name to console         */
#define kCLI_FLAG_KILL                              0x80000000 /* exit thread                       */


/* 命令行键盘输入定义 */

/* 
   vt100 cursor movement escape codes
   - abstract out to support other term types!
*/

#define kCLI_VTTERM_UP      "\x1B[%dA"
#define kCLI_VTTERM_DN      "\x1B[%dB"
#define kCLI_VTTERM_RT      "\x1B[%dC"
#define kCLI_VTTERM_LT      "\x1B[%dD"

/* telnet screen commands */
#define kCLI_TELNET_BACKSPACE   "\b \b"
#define kCLI_TELNET_CLEAR       "\33[2J"

/* 
 *   cursor/keyboard escape codes  
*/
#define kCLI_ESC_CURSOR   0x5b
#define kCLI_CURSOR_UP    0x41
#define kCLI_CURSOR_DOWN  0x42
#define kCLI_CURSOR_RIGHT 0x43
#define kCLI_CURSOR_LEFT  0x44

/* IOS-like escape codes */
#define kCLI_WORD_PREV               'b'
#define kCLI_WORD_NEXT               'f'
#define kCLI_WORD_UPPERCASE_TO_END   'c'
#define kCLI_WORD_DELETE_TO_END      'd'
#define kCLI_WORD_LOWERCASE_TO_END   'l'

#define kKEY_ESC_OFFSET         ((unsigned char)(0xA0))
#define kKEY_HELP               ((unsigned char) '?')
#define kKEY_MOVE_UP            ((unsigned char)(0x10))
#define kKEY_MOVE_DOWN          ((unsigned char)(0x0E))
#define kKEY_MOVE_LEFT          ((unsigned char)(0x02))
#define kKEY_MOVE_RIGHT         ((unsigned char)(0x06))
#define kKEY_LINE_START         ((unsigned char)(0x01))
#define kKEY_LINE_END           ((unsigned char)(0x05))
#define kKEY_DELETE_TO_END      ((unsigned char)(0x0B))
#define kKEY_DELETE_CHAR        ((unsigned char)(0x04))
#define kKEY_DELETE_FROM_START  ((unsigned char)(0x15))
#define kKEY_DELETE_WORD_START  ((unsigned char)(0x17))
#define kKEY_REFRESH_DISPLAY    ((unsigned char)(0x0C))
#define kKEY_TRANSPOSE          ((unsigned char)(0x14))
#define kKEY_END_OF_ENTRY       ((unsigned char)(0x1A))
#define kKEY_WORD_PREV          ((unsigned char)(kKEY_ESC_OFFSET) + 'B')
#define kKEY_WORD_NEXT          ((unsigned char)(kKEY_ESC_OFFSET) + 'F')
#define kKEY_UPPERCASE          ((unsigned char)(kKEY_ESC_OFFSET) + 'C')
#define kKEY_DELETE_WORD_END    ((unsigned char)(kKEY_ESC_OFFSET) + 'D')
#define kKEY_LOWERCASE          ((unsigned char)(kKEY_ESC_OFFSET) + 'L')
#define kKEY_DELETE             ((unsigned char)(0x7F))
#define kKEY_BREAK              ((unsigned char)(0x03))


#define LEAST_RECENT_HIST(x) (CLI_MAX(x->iNumCmds - x->iMaxHistCmds + 1, 0))
#define HIST_BUFFER_FULL(x)  (x->bufferIndex == (x->iMaxHistCmds - 1))
#define HistBuffPtr(x, y)    ((_CHAR *) &(x->pHistBuff[y].histCmd))


#define __ENABLE_CUSTOM_STRUCT__


#define     CLI_MAX_CMD_LEN                  60
#define     CLI_MAX_CMD_HELP_LEN        1000
#define     CLI_MAX_ARG_LEN              30
#define     CLI_MAX_ARG_NUM              10
#define     CLI_MAX_INPUT_LEN               200
#define     CLI_REQ_EXPIRE_TIME       5000  //five seconds

#define     CLI_MAX_CMD_NUM                 50
#define     K_MAX_ENV_SIZE                      19

#define kenv_HTTP_VERSION       14

/* cache object access */
#define K_CACHE_NO            0
#define K_CACHE_READ          1
#define K_CACHE_WRITE         2
#define K_CACHE_READWRITE     (K_CACHE_READ | K_CACHE_WRITE)

#define K_TCP_LISTEN_BACKLOG 20
#define K_TCPD_TASKNAME_LENGTH    16

/* Transmission Flow Constants */
#define K_FLOW_OFF    0
#define K_FLOW_ON      1


#define PARA_ERROR(x)  \
    {\
    CLI_EXT_WriteStrLine(x,"无效参数");\
    return;\
    }

#define CLI_ENV GEN_ENVIRON


/*
* 访问宏定义 
*/

#define CLIENV(pEnv)    (pEnv)->pCli

#define CLI_EnableFeature(pEnv, x)              (CLIENV(pEnv)->cliFlags |=  x)
#define CLI_DisableFeature(pEnv, x)             (CLIENV(pEnv)->cliFlags &= ~x)
#define CLI_IsEnabled(pEnv, x)                  (CLIENV(pEnv)->cliFlags &   x)
#define CLI_NotEnabled(pEnv, x)                 (!(CLIENV(pEnv)->cliFlags & x))

#define MCONN_GetSubOption(pEnv)                (CLIENV(pEnv)->subOption)
#define MCONN_SetSubOption(pEnv, X)             (CLIENV(pEnv)->subOption = X)
#define MCONN_OptBufferPtr(pEnv)                    (_CHAR *) &(CLIENV(pEnv)->optBuffer)
#define MCONN_GetOptBufferIndex(pEnv)           (CLIENV(pEnv)->optBufferIndex)
#define MCONN_SetOptBufferIndex(pEnv, X)        (CLIENV(pEnv)->optBufferIndex = X)
#define MCONN_GetRecvState(pEnv)                (CLIENV(pEnv)->recvState)
#define MCONN_SetRecvState(pEnv, X)             (CLIENV(pEnv)->recvState = X)
#define MCONN_TermType(pEnv)                    (CLIENV(pEnv)->terminalType)

#define MCHAN_Env(pChan)                        ((GEN_ENVIRON *)(pChan->env))
#define MCHAN_CliEnv(pChan)                     (MCHAN_Env(pChan)->pCli)

#define MMISC_GetOptHandled(pEnv)               (CLIENV(pEnv)->optHandled)
#define MSCRN_GetWidth(pEnv)                    (CLIENV(pEnv)->screenWidth)
#define MSCRN_GetHeight(pEnv)                   (CLIENV(pEnv)->screenHeight)
#define MSCRN_SetWidth(pEnv,  x)                (CLIENV(pEnv)->screenWidth  = x)
#define MSCRN_SetHeight(pEnv, y)                (CLIENV(pEnv)->screenHeight = y)
#define MEDIT_GetLength(pEnv)                   (CLIENV(pEnv)->cmdEditInfo.lineLength)
#define MEDIT_GetCursor(pEnv)                   (CLIENV(pEnv)->cmdEditInfo.cursorPos)
#define MEDIT_SetLength(pEnv, x)                (CLIENV(pEnv)->cmdEditInfo.lineLength = x)
#define MEDIT_SetCursor(pEnv, y)                (CLIENV(pEnv)->cmdEditInfo.cursorPos  = y)
#define MEDIT_GetKeyStat(pEnv)                   (CLIENV(pEnv)->cmdEditInfo.keyState)
#define MEDIT_SetKeyStat(pEnv, x)                   (CLIENV(pEnv)->cmdEditInfo.keyState = x)

#define MEDIT_BufPtr(pEnv)                          (CLIENV(pEnv)->cmdEditInfo.inputArray)
#define MEDIT_Prompt(pEnv)                          (CLIENV(pEnv)->prompt)
#define MEDIT_PromptLen(pEnv)                   (CLIENV(pEnv)->promptLength)
#define MEDIT_EditInfoPtr(pEnv)                 (&(CLIENV(pEnv)->cmdEditInfo))

#define MHIST_History(pEnv)                   (&(CLIENV(pEnv)->histInfo))
#define MHIST_TempBuf(hist)                     (hist->tempHist)

#define MMISC_GetChannel(pEnv)                  (CLIENV(pEnv)->pChannel)
#define MCONN_GetWriteHandle(pEnv)              (CLIENV(pEnv)->pCliWriteHandle)
#define MCONN_GetReadHandle(pEnv)               (CLIENV(pEnv)->pCliReadHandle)
#define MCONN_SetWriteHandle(pEnv, X)           (CLIENV(pEnv)->pCliWriteHandle = X)
#define MCONN_SetReadHandle(pEnv, X)            (CLIENV(pEnv)->pCliReadHandle = X)
#define MCONN_GetConnType(pEnv)                 (CLIENV(pEnv)->typeConn)
#define MCONN_SetConnType(pEnv, X)              (CLIENV(pEnv)->typeConn = X)
#define MCONN_GetSock(pEnv)                     (CLIENV(pEnv)->pChannel->sock)
#define MMISC_GetRootParam(pEnv)                (CLIENV(pEnv)->pParamRoot)
#define MMISC_SetRootParam(pEnv,pList)          (CLIENV(pEnv)->pParamRoot = pList)
#define MMISC_GetInput(pEnv)                (CLIENV(pEnv)->chMore)
#define MMISC_SetInput(pEnv, depth)         (CLIENV(pEnv)->chMore = depth)
#define MMISC_GetFsmId(pEnv)                (CLIENV(pEnv)->fsmId)
#define MMISC_SetFsmId(pEnv, x)         (CLIENV(pEnv)->fsmId = x)

#define MMISC_GetAccess(pEnv)                   (pEnv->UserLevel)
#define MMISC_SetAccess(pEnv, access)           (pEnv->UserLevel = access)
#define MMISC_PrevRootPtr(pEnv)                 (CLIENV(pEnv)->pPrevRoot)
#define MMISC_Login(pEnv)                       (CLIENV(pEnv)->login)
#define MMISC_Passwd(pEnv)                       (CLIENV(pEnv)->passwd)

#define MMISC_AliasCmd(pEnv)                    (CLIENV(pEnv)->aliasTable.aliasCmd)
#define MMISC_AliasPtr(pEnv)                   &(CLIENV(pEnv)->aliasTable)
#define MMISC_GetErrorPos(pTokens)              (pTokens->errorPos)
#define MMISC_SetErrorPos(pTokens, x)           (pTokens->errorPos = x)
#define MMISC_GetErrorText(pEnv)                (CLIENV(pEnv)->errorText)
#define MMISC_SetErrorText(pEnv, msg)            STRNCPY(CLIENV(pEnv)->errorText, msg, kCLI_ERROR_TEXT_SIZE)
#define MMISC_ClearErrorText(pEnv)              (*(CLIENV(pEnv)->errorText) = '\0')
#define MMISC_GetNodePrompt(x)                  (NULL != x ? x->pPrompt : NULL)
#define MMISC_GetLoginStat(pEnv)                 (CLIENV(pEnv)->loginStat)
#define MMISC_SetLoginStat(pEnv, x)                 (CLIENV(pEnv)->loginStat = x)

#define MEDIT_CopyFromInput(pEnv, dest) \
        strcpy(dest, CLIENV(pEnv)->cmdEditInfo.inputArray)
#define ENVOUT(pEnv)                ((pEnv)->output)
#define MMISC_OutputPtr(pEnv)     (&(ENVOUT(pEnv)))
#define MMISC_OutputBuffer(pEnv)    (ENVOUT(pEnv).pOutput)


/* 操作宏定义*/
#define NULL_STRING(x)          ((NULL == x) || ('\0' == *x))
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))
#define DIGIT_TO_CHAR(Digit)    ((_CHAR)(Digit - '0'))

#define CLI_MIN(x, y)   ( (x < y) ? (x) : (y) )
#define CLI_MAX(x, y)   ( (x > y) ? (x) : (y) )

#define TOUPPER(x)  ( (('a' <= x) && ('z' >= x)) ? (x - 'a' + 'A') : x )
#define TOLOWER(x)  ( (('A' <= x) && ('Z' >= x)) ? (x - 'A' + 'a') : x )


/* Free and null pointer to allocated memory */
#define FREEMEM(x) {if (NULL != x) {RC_FREE(x); x = NULL;}}

#if !defined (__LITTLE_ENDIAN_SYSTEM__) || defined(__DISABLE_LITTLE_ENDIAN_CONVERSION__)

#define HTON2(x) (x)
#define NTOH2(x) (x)
#define HTON4(x) (x)
#define NTOH4(x) (x)
    
#else

#define HTON2(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0x00FF)) 
#define NTOH2(x) HTON2(x)
#define HTON4(x) (   (( (x) << 24 ) & 0xFF000000) | (( (x) << 8 ) & 0x00FF0000) \
                   | (( (x) >> 8 ) & 0x0000FF00) | (( (x) >> 24 ) & 0x000000FF) )
#define NTOH4(x) HTON4(x)

#endif

#ifndef CLI_AUTH_CALLBACK_FN
#define CLI_AUTH_CALLBACK_FN    CLI_TASK_ValidateLogin
#endif

#ifdef WINDOWS
#define GETCHAR() getchar() 
#else
#define GETCHAR() getchar()
#endif

#ifndef RL_STATIC
#define RL_STATIC static
#endif

#ifndef TELNET_SEND_FN
#define TELNET_SEND_FN  CLI_TELNET_Send
#endif

#ifndef TELNET_RECV_FN
#define TELNET_RECV_FN  CLI_TELNET_Recv
#endif

#define GET_PRINT_FLAG(out, flag)   (0 != (out->flags & flag))
#define SET_PRINT_FLAG(out, flag)   (out->flags |= flag)
#define CLR_PRINT_FLAG(out, flag)   (out->flags &= ~flag)
#define WHITE_SPACE(text)           ((' ' == text) || (kCR == text) || (kLF == text) || (kCHAR_NULL == text))

#define kPRINT_FLAG_NOPRINT  0x00000001 /* ignore print requests        */

#undef kEOL

#ifdef __EOL_USES_LF__
#define kEOL "\n"
#endif

#ifdef __EOL_USES_CR__
#define kEOL "\r"
#endif

#ifndef kEOL
#define kEOL "\r\n"
#endif

#define kEOL_SIZE (sizeof(kEOL) - 1)

enum QueueState 
{
    QUEUE_None,
    QUEUE_Empty,
    QUEUE_Opposite
};

enum    RL_ThreadStates     
{
    kThreadCreate,  
    kThreadIdle,     
    kThreadSnmpSync, 
    kThreadWorking, 
    kThreadFinished, 
    kThreadDead 
};

/*
#define kKEY_CONTINUE           '\\'
#define kKEY_SEPARATOR          ';'
**/
typedef enum KEY_STATE 
{
    KEY_STATE_INVALID, 
    KEY_STATE_DATA,
    KEY_STATE_ESC,
    KEY_STATE_CURSOR
} KEY_STATE;


/* 
*   类型定义 
*/

/* 命令模式,每个命令均位于特定的模式下*/

typedef enum cliMode  
{
    sysMd = 0,
    sysL2,


    /*IP 设置
    ipset,*/

	
    /* 
    * 平台
    */

    plat,


    /*
    * 传输层
    *
    */

    transpt,

    
    /* 消息路由*/

    route,


      /*IP 模式 */

       ip,

    /* v5 lapd */

    lapd,

    tlpCli,
    /*
    * 资源层
    *
    */

    res,

    /*
    * 协议层
    *
    */

    protocol,
    
        /* V5 模式*/

       V5Md,


    /*
    * 业务层
    *
    */

    service,

      /* 呼叫控制*/

        cc,

    /* 短消息*/

        sms,

    /* 移动管理*/

        mm,

    /* 操作维护*/

       oam,

    /* 标准接口
      *
      */

        show,

        set,

     
    ExitCliShellMd
    
}CLI_MODE;


typedef _SHORT  Access;

#pragma pack(1)

//-------------------------------结构定义------------------------------------


//消息所有者属性结构
typedef struct
{
    _UCHAR   ucModId;           //模块号
    _UCHAR   ucFId;             //功能块号
    _USHORT  usFsmId;           //内部连接号
}MSG_OWNER;

//公共消息头结构
typedef struct tagCliCommonHeader
{
    MSG_OWNER   stReceiver;     //接收者
    MSG_OWNER   stSender;       //发送者
    _USHORT     usMsgId;        //消息号
    _USHORT     usMsgLen;       //消息长度
}CliCOMMON_HEADER;

//通用简单消息结构
typedef struct tagCliCommonMessage
{
    CliCOMMON_HEADER stHeader;
    _UCHAR ucBuffer[1];
}CliCOMMON_MESSAGE;



//OAM 消息
typedef struct tagOamMessage
{
    CliCOMMON_HEADER stHeader;
    _UCHAR ucBuffer[CLI_MAX_CMD_HELP_LEN];
}OAM_MESSAGE;


/* CMD 参数*/

typedef struct cmdArg
{
    _INT        siIdx;
    _INT        siArgcnt;
    _CHAR     pscArgval[CLI_MAX_ARG_NUM][CLI_MAX_ARG_LEN];    
}CMD_ARG;

/* 单条CMD  结构*/

typedef struct cmdDatas 
{
    _INT        siFlag;    
    
     //CMD 处理函数
    _VOID*   pCmdHandler;

     //CMD 名称
    _CHAR   pscCmdName[CLI_MAX_CMD_LEN];

     // CMD 帮助字符串
    _CHAR   *pscCmdHelpStr;

     // CMD 参数说明
    _CHAR   *pscParaHelpStr;
}CMD_DATAS;

/* 命令行结构*/

typedef struct cliShellCmd
{
      /* 模式代表一个CMD 层次*/
    CLI_MODE          cliMode;
      _CHAR             pscCursor[CLI_MAX_ARG_LEN];   //模式提示符

       //该配置模式下的command列表
        CMD_DATAS       *pCmdLst;   

    CMD_ARG           cmdArg;

    _ULONG          ulPara0;
    _ULONG          ulPara1;
        
}CLI_SHELL_CMD;


typedef struct cliApCmds
{
    //IP 设置模式
    CMD_DATAS   *pIPSetCmds;

    // 平台模式用户命令
    CMD_DATAS   *pPlatCmds;

    // 路由模式
    CMD_DATAS   *pRouteCmds;

    // IP 模块命令
    CMD_DATAS   *pIpCmds;

    // LAPD 模块命令
    CMD_DATAS   *pLapdCmds;

    // LAPD 模块命令
    CMD_DATAS   *pTlpCmds;

    // 资源命令
    CMD_DATAS   *pResCmds;

    // V5模块命令
    CMD_DATAS   *pV5Cmds; 

    //呼叫控制模块命令
    CMD_DATAS   *pCcCmds; 

    //短信模块命令
    CMD_DATAS   *pSmsCmds; 

    //移动管理模块命令
    CMD_DATAS   *pMmCmds; 

    //操作维护模块命令
    CMD_DATAS   *pOamCmds;
    
}CLI_AP_CMDS;


typedef struct OPTION_STATE
{
    _CHAR     name;
    _UCHAR   count;
    _UCHAR   optState;
    _UCHAR   queueState;
} OPTION_STATE;

typedef struct PAIR_STATE 
{
    _UCHAR     option;
    _CHAR       desired;
    OPTION_STATE host;
    OPTION_STATE client;
} PAIR_STATE;


/* 
* TELNET 关联结构
*/

typedef struct CLI_LIST
{
    _VOID            *pObject;       /* pointer to data object  */
    struct  CLI_LIST    *pNext;         /* next object in the LIST */

} CLI_LIST;

typedef struct LINE_OUT 
{
    _ULONG flags;                       /* output settings                      */
    _INT  maxSize;                      /* maximum buffer size                  */
    _INT  length;                           /* length of buffer contents            */
    _INT  offset;                           /* offset to text not yet printed       */
    _INT  lineCount;                    /* number of lines printed this time    */
    _CHAR  *pOutput;                   /* output buffer                        */
} LINE_OUT;

#if 0
typedef struct UDP_PARAMS
{
    _CHAR *pRecPacket;
    _CHAR *pSendPacket;
    _ULONG clientAddr;
    _ULONG sendPacketLength;
    _ULONG recvPacketLength;
    _USHORT clientPort;

} UDP_PARAMS;
#endif

typedef struct CACHE_HANDLE                  /* basename:  cac */
{
    struct  CLI_LIST    *p_lstCacheObjects;     /* next object in the LIST */
    Access          AccessType;                     /* cache access (Read/Write) */

} CACHE_HANDLE;

typedef struct CMD_EDITINFO 
{
    _SHORT  lineLength;     /* character count in line buffer     */
    _SHORT  cursorPos;      /* offset of cursor into buffer       */
    _SHORT  termX;          /* horizontal pos of cursor           */
    _SHORT  termY;          /* vertical pos of cursor             */
    _SHORT  startCol;       /* track before exec handler          */
    _SHORT  startRow;       /* track before exec handler          */
    KEY_STATE    keyState;
    _CHAR     inputArray[CLI_MAX_CMD_LEN];
} CMD_EDITINFO;

typedef struct CmdHistBuff 
{
    _CHAR  histCmd[CLI_MAX_CMD_LEN]; /* the command */
} CmdHistBuff;

/*-----------------------------------------------------------------------*/

typedef struct HistInfo 
{
    _INT bufferIndex;                /* points to [iNumCmds]              */
    _INT iMaxHistCmds;             /* max num hist cmds                 */
    _INT iCurHistCmd;                  /* current hist cmd                  */
    _INT iNumCmds;                   /* total number of commands issued   */
    _CHAR  tempHist[CLI_MAX_CMD_LEN]; /* current command when scrolling    */
    CmdHistBuff *pHistBuff;
} HistInfo;


typedef struct  GEN_ENVIRON                     /* (basename:  env) */
{
    struct CACHE_HANDLE* phCacheHandle;
    Access              UserLevel;

    _ULONG              IpAddr;
    _INT                   clientIndex;

    _CHAR             PostValid;
    _CHAR*              PostBuffer;
    _ULONG             PostBufferLen;

#ifdef __ENABLE_CUSTOM_STRUCT__
    _VOID*               pCustomData;            /* custom session data */
#endif

   struct  CLI_INFO   *pCli;
    LINE_OUT           output;
} GEN_ENVIRON;

typedef _INT WriteHandle(GEN_ENVIRON *, _CHAR *pBuf, _INT  BufSize);
typedef _INT ReadHandle(GEN_ENVIRON *, _UCHAR, _CHAR *pBuf, _INT *bytesReturned);

typedef struct CLI_INFO
{
    _ULONG                            cliFlags;
    _INT                              chMore;
    _CHAR                           loginStat;
    _CHAR                             fsmId;
    struct COM_CHAN           *pChannel;
    struct GEN_ENVIRON        *pEnvironment;
    _CHAR               login[kCLI_MAX_LOGIN_LEN];
    _CHAR               passwd[kCLI_MAX_PASSWORD_LEN];
    _ULONG              promptLength;
    CMD_EDITINFO         cmdEditInfo;
    _CHAR               errorText[kCLI_ERROR_TEXT_SIZE];
    PAIR_STATE           optHandled[kCLI_MAX_OPT_HANDLED];
    _UCHAR             subOption;
    _UCHAR               recvState;
    _ULONG        typeConn;
    WriteHandle        *pCliWriteHandle;
    ReadHandle         *pCliReadHandle;
    _INT              optBufferIndex;
    _CHAR               optBuffer[kCLI_OPT_BUF_SIZE];
    _CHAR               terminalType[kCLI_TERM_TYPE_SIZE];
    _SHORT            screenWidth;
    _SHORT            screenHeight;
    CLI_SHELL_CMD     cliShellCmd;
    _VOID               *pCustom;  /* customer defined */
    HistInfo            histInfo;

} CLI_INFO;


/*-----------------------------------------------------------------------*/

typedef struct COM_CHAN
{
    _CHAR                     InUse;      /* if TRUE, there is a request on this com channel ... */
    _INT                      ThreadState;
    _ULONG                      IpAddr;
    unsigned int                index;

    _VOID*                       env;

} COM_CHAN;

#if 0
//-------------------------------枚举定义------------------------------------
//功能块初始化步骤定义
typedef enum 
{
    STEP_LOAD_CFG,          //加载配置(从Flash、Nvrom中读取数据到变量中)
    STEP_MALLOC,            //申请内存,注册哈希表
    STEP_INITIAL,           //初始化全局变量
    STEP_TIMER_REG,         //注册定时器个数
    STEP_CLI_REG,           //注册命令行接口
    STEP_STANDBY_REG,       //注册备份接口
    STEP_KICKOFF,           //开工(主要做初始化的总结性工作,例如根据定时器的个数进行定时器的总注册)
    STEP_RESET_TIMER,       //启动定时器
    STEP_RESET_BUFF,        //最后阶段的初始化

    MAX_STEP
}INIT_STEP;





//功能块ID的定义
typedef enum
{
    // 0 ~ 4
    FID_SYS         = 0,
    FID_TIMER,
    FID_IP,
    FID_CLOCK,
    FID_COMM,           //板间通信处理功能块(仅当在PC机上模拟板间通信时使用)

    // 5 ~ 9
    FID_CLI,
    FID_DTCP,
    FID_SYN,            //数据同步功能块
    FID_TEST,
    FID_DEBUG,

    // 10 ~ 14
    FID_MNT,
    FID_CC,
    FID_PG,
    FID_SMS,
    FID_MM,

    // 15 ~ 19
    FID_OAM,
    FID_SA,
    FID_SAB,  
    FID_PSTN,
    FID_BCC,

    // 20 ~ 24
    FID_SRC,
    FID_NET,
    FID_NMS,
    FID_NMSMNT,
    FID_LAPD,

    // 25 ~ 29
    FID_DRIVER,
    FID_MNT_SERVER,     //monitor server


    // SA接入模块内的功能块定义，李常青，2004/11/22
    // 根据原来的创建的任务相应一个功能块，
    // 接收DRAM，网管有关的消息原tRecUserMsgTask任务由上面的FID_SA对应，
    // FID_SA为唯一的接收外界模块的对外功能块号，即外界模块向SA模块发送
    // 消息时添的功能块号；
    FID_SA_ALARM,
    FID_SA_L1,
    FID_SA_CTRL,

    // 30 ~ 34
    FID_SA_LCTRL,
    FID_SA_PSTN,
    FID_SA_BCC,
    FID_SA_PROTECT,
    FID_SA_MAP,

    // 35 ~ 39
    FID_SA_SYSBACK,
    //  SA接入模块内的功能块定义完毕



    FID_TEST_UDP_1,
    FID_TEST_UDP_2,

    //模拟L3，测试用
    FID_L3_1,
    FID_L3_2,

    // 40 ~ 44
    FID_TST_DTCP,

    FID_ETN_TRAN,   //以太网消息透明传输服务功能块
    FID_BCSMS,
    FID_CSS_SYS,     // CSS 系统层模块
    FID_CSS_L3,      // CSS L3层模块
    FID_CSS_NMS,     // CSS NMS层模块
    FID_CSS_MCS,     // CSS 客户端 fid

    FID_CSS_L3_PSTN,
    FID_CSS_L3_BCC,
    FID_CSS_L3_8181,

    FID_PPPOE,//S501
    FID_TLP,   //s501
    FID_INTF2GWF,
    FID_INTF2TLP,
    FID_MUX,
    FID_HDLC,
    FID_GWF,
    MAX_FID
}FID_E;

//定时器功能块和其他功能块的消息
typedef enum
{
    TIMER_MSG_ID,           //定时器发给各功能块的通用消息ID

    MAX_TIMER_MSG
}TIMER_MSG_E;


#endif
/* 
*   全局变量引用
*/

extern CMD_DATAS   sysCmds[]; 
//extern _INT SYS_IPMsgProc(LONG_MESSAGE* pstMsg);

/* 
*   函数声明 
*/

_ULONG CLI_IniProc();
_INT CLI_MsgProc(CliCOMMON_HEADER* pstHeader);
_INT CLI_MsgTask();
_VOID sendCloseReq(_INT connId);

_VOID cliShellTsk(_VOID);
_INT ExcuteCommand(CLI_ENV *pCliEnv);
_INT ParseCommand(CLI_ENV *pCliEnv);
_INT GetCommand(_VOID);

_VOID initCmdTree();
_VOID initCliMem();
_VOID initCliApCmd();
_VOID initApCmd(CMD_DATAS* pDstCmd, CMD_DATAS* pSysCmd, CMD_DATAS* pApCmd);

_VOID InitPrompt(CLI_ENV* pCliEnv);
_VOID modeSwitch(CLI_ENV* pCliEnv, _INT siCurMode,_INT siDstMode, _INT siPara0,_INT siPara1);

//_INT usrLogin();
_INT checkPasswd(_UCHAR* usr, _UCHAR* pwd);


// 命令树处理函数

_VOID cmdHelp(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdShow(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdSet(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdIPShow(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdIPSet(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdShellParse(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdDiagShow(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdCls(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);

_VOID cmdEnterL2Shell(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdL2Exit(CLI_ENV* pCliEnv, _INT siArgc , _CHAR **ppArgv);
_VOID cmdL2ShellParse(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);

// 一级模式
_VOID cmdPlat(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdTspt(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdRes(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdPrtl(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdSvc(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdMPUIPSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdRnmsIPSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdDebugLevelShow(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdDebugLevelSet(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdDebugOpen(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdDebugClose(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);

/*lijinan 090108 for telnet all cmd*/
_VOID cmdOpen(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID cmdClose(CLI_ENV* pCliEnv, _INT siArgc,_CHAR **ppArgv);
	

//二级模式

_VOID cmdRoute(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv); // 传输层
_VOID cmdIp(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdLapd(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdTlp(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);

_VOID cmdV5(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);


_VOID cmdCc(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv); // 传输层
_VOID cmdMm(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdSms(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);
_VOID cmdOam(CLI_ENV* pCliEnv, _INT siArgc, _CHAR **ppArgv);

_VOID cmdQuit(CLI_ENV* pCliEnv, _INT siArgc , _CHAR **ppArgv);
_VOID cmdExit(CLI_ENV* pCliEnv, _INT siArgc , _CHAR **ppArgv);

// 输入输出函数

_VOID StrToUp(_CHAR * s);
_VOID int_to_ip(_UINT addr, _CHAR *ip);
_UINT ip_to_int(const _CHAR *cp, _UINT *pnet_addr);



// 结构操作函数

_INT     ENVIRONMENT_Construct(GEN_ENVIRON **pp_envInit);
 _INT     ENVIRONMENT_Destruct (GEN_ENVIRON **pp_envTemp);

/* Environment Variables */
  _VOID CLI_DB_DestroyEnvironment(CLI_ENV *pCliEnv);

_VOID     CLI_EXT_DeleteChar(CLI_ENV *pCliEnv);
_VOID     CLI_EXT_EraseLine(CLI_ENV *pCliEnv);
_VOID     CLI_EXT_EnablePrint(CLI_ENV *pCliEnv, _CHAR enable);
_VOID     CLI_EXT_FreeOutput(CLI_ENV *pEnv);
_CHAR    CLI_EXT_GetPriorChar(CLI_ENV *pCliEnv);
_VOID     CLI_EXT_InitCommandLine(CLI_ENV *pCliEnv);
_INT        CLI_EXT_InitOutput(CLI_ENV *pEnv, _INT size);
_VOID     CLI_EXT_InsertText(CLI_ENV *pCliEnv, _CHAR *text, _INT length);
_VOID     CLI_EXT_LineEnd(CLI_ENV *pCliEnv);
_VOID     CLI_EXT_LineStart(CLI_ENV *pCliEnv);
_VOID     CLI_EXT_LocalEcho(CLI_ENV *pCliEnv, _CHAR enable);
_VOID     CLI_EXT_MoveCursor(CLI_ENV *pCliEnv, _SHORT offset);
_VOID     CLI_EXT_PrintString(CLI_ENV *pCliEnv, _CHAR *pString, _INT length, _CHAR fill);
_INT        CLI_EXT_Put(CLI_ENV *pCliEnv, _CHAR *pBuf, _INT bufLen);
_INT        CLI_EXT_PutStr(CLI_ENV *pCliEnv, _CHAR *pBuf);
_INT        CLI_EXT_ReadCmd(CLI_ENV *pCliEnv);
_VOID     CLI_EXT_ResetOutput(CLI_ENV *pCliEnv); 
_VOID     CLI_EXT_SetCursor(CLI_ENV *pCliEnv, _SHORT position);
_INT        CLI_EXT_Write(CLI_ENV *pCliEnv, _CHAR *pBuf, _INT bufLen);
_INT        CLI_EXT_WriteStr(CLI_ENV *pCliEnv, _CHAR *pBuf);
_INT        CLI_EXT_WriteStrLine(CLI_ENV *pCliEnv, _CHAR *pBuf);
_INT CLI_CMD_Clear(CLI_ENV *pCliEnv);

_VOID CLI_TASK_PrintError(CLI_ENV *pCliEnv, _INT status);
_INT Cache_Construct(CACHE_HANDLE **pph_cacHandle, Access AccessType);
 _INT CLI_UTIL_Init(CLI_ENV *pCliEnv);
_CHAR CLI_TASK_ValidateLogin(_CHAR *pLogin, _CHAR *pPassword, Access *pAccLvl/* 用户级别暂未使用*/);
_INT CLI_TASK_Login(CLI_ENV *pCliEnv);

 _INT CLI_DB_InitEnvironment(CLI_ENV **ppCliEnv, COM_CHAN *pChannel);

_VOID *RC_MALLOC(_ULONG memSize);
_VOID RC_FREE(_VOID *pBuffer);
_VOID *RC_CALLOC(_UCHAR num, _ULONG size);

CLI_LIST*    List_Construct      (void);

_INT CLI_EXT_ReadCh(CLI_ENV *pCliEnv, _UCHAR* pOrgCh, _USHORT orgLen);

_VOID CLI_EXT_NEWLINE(CLI_ENV* pCliEnv);

_INT CLI_HIST_InitHistInfo(CLI_ENV *pCliEnv);
_VOID CLI_HIST_AddHistLine (CLI_ENV *pCliEnv);
_VOID CLI_HIST_Scroll(CLI_ENV *pCliEnv, _INT offset);

_INT     CLI_TELNETD_BroadcastMessage(_CHAR *pMessage, Access authLevel);
_INT CLI_EXT_Printf(CLI_ENV* pCliEnv, _CHAR* pFmt, ...);


#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */
