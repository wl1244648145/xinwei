/************************************************************************
*   版权所有：北京信威通信股份有限公司
*   文件名：ControlMain.c
*	文件描述: IP模块主处理部分
*   修改记录：
*   	1.  2006.7.5   Li Wei   创建
************************************************************************/
#include "sysos.h"
#include "msg.h"
#include "ipTask.h"


#ifdef THIS_FILE_ID
#undef THIS_FILE_ID
#endif

#define THIS_FILE_ID FILE_IPTASK_C

SYS_IP_DATA *m_pTelnet = NULL;
SYS_TASK_ID  TelnetSvr=NULL;
extern _INT IoChangeFlag;
extern _INT HoldL2Flag;
extern _INT RunShellFlag;
extern SYS_SOCKET_ID CliSock;
extern SYS_SOCKET_ID PipeFd;
extern SYS_SOCKET_ID CliPipeFd;

extern _VOID CLI_MsgTask();
extern _VOID RtnStdIO();
extern 
STATUS SendCommandToL2(char *bufPtr, int lineLen);

_UCHAR sendTelnetBuf[MAX_IP_DATA_LEN]; //为了处理CRLF而增加的缓冲区
_UCHAR ucParaBuf[MAX_IP_DATA_LEN];
_USHORT ucParaLen = 0;


/****************************************************************************************
 函数名称：    SYS_IPSendMsg2User												
 函数的功能:   发送消息到上层								
 全局变量：																			
 输入参数：	
                                pstCcb:                 IP控制块
                                usMsgId:               消息号
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_TelnetCloseClient(SYS_IP_CCB *pstCcb)
{
    _USHORT usLength;
    LONG_MESSAGE *pstMsg;
    
    

    pstMsg = (LONG_MESSAGE *)m_pTelnet->ucBuffer;
    
    usLength = pstMsg->stHeader.usMsgLen;
    pstMsg->stHeader.ucSrcTaskNo = pstCcb->ucProvider;
    pstMsg->stHeader.ucDstTaskNo= pstCcb->ucUser;
    pstMsg->stHeader.usMsgId = MSG_IP_DATAIND;

    pstMsg->stHeader.usMsgLen = 5;
    pstMsg->ucBuffer[0] = pstCcb->ucIndex;
    pstMsg->ucBuffer[1] = 'e';
    pstMsg->ucBuffer[2] = 'x';
    pstMsg->ucBuffer[3] = 'i';
    pstMsg->ucBuffer[4] = 't';
    
    SendMsgTelnet(pstMsg);


    

    return SUCC;
}


/****************************************************************************************
 函数名称：    SYS_IPSendMsg2User												
 函数的功能:   发送消息到上层								
 全局变量：																			
 输入参数：	
                                pstCcb:                 IP控制块
                                usMsgId:               消息号
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPSendMsg2User(SYS_IP_CCB *pstCcb, _USHORT usMsgId)
{
    _USHORT usLength;
    LONG_MESSAGE *pstMsg;
    

    pstMsg = (LONG_MESSAGE *)m_pTelnet->ucBuffer;
    
    usLength = pstMsg->stHeader.usMsgLen;
    pstMsg->stHeader.ucSrcTaskNo = pstCcb->ucProvider;
    pstMsg->stHeader.ucDstTaskNo= pstCcb->ucUser;
    pstMsg->stHeader.usMsgId = usMsgId;
    
    if (MSG_IP_DATAIND != pstMsg->stHeader.usMsgId)
    {
	if(pstCcb->stMyAddr.port == TELNET_PORT)
	{
		pstMsg->stHeader.usMsgLen = 1;
		pstMsg->ucBuffer[0] = pstCcb->ucIndex;
	       SendMsgTelnet(pstMsg);
	       //MPUMsgSend(pstMsg,NO_WAIT);
	}
       
    }
    else
    {

        
        
        pstMsg->stHeader.usMsgLen = usLength;
        if(pstCcb->stMyAddr.port == TELNET_PORT)
        {
            pstMsg->stHeader.usMsgLen += 1;
            pstMsg->ucBuffer[usLength] = pstCcb->ucIndex;
            SendMsgTelnet(pstMsg);
            //MPUMsgSend(pstMsg,NO_WAIT);
        }
        
        
    }

    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPClearSelect												
 函数的功能:   清除一个SOCKET监视								
 全局变量：																			
 输入参数：    pstCcb:                 IP控制块
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPClearSelect(SYS_IP_CCB *pstCcb)
{
    if((pstCcb->ucIndex >= IP_FSM_CLI_FIRST_CLIENT)&&(pstCcb->ucIndex < IP_FSM_CLI_SERVER))
    {
        SYS_IPSendMsg2User(pstCcb, MSG_IP_CLOSEIND);
    }

    FD_CLR(pstCcb->sock, &m_pTelnet->fdRead);
    FD_CLR(pstCcb->sock, &m_pTelnet->fdWrite);
    FD_CLR(pstCcb->sock, &m_pTelnet->fdException);
    SYS_SockClose(pstCcb->sock);
    pstCcb->sock = 0;

    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPCloseSClient												
 函数的功能:    关闭一个服务器引发的客户连接								
 全局变量：																			
 输入参数：    pstCcb:          IP控制块
                               iDir:               发起方向,0:用户发起,1:网络发起
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPCloseSClient(SYS_IP_CCB *pstCcb, _INT iDir)
{    
    //if (SYS_IP_STATE_RUNNING != pstCcb->ucState)//如果不在运行状态下,直接返回
    //    return FAIL;
//    if (0 != iDir)//如果不是用户发起的
//    {
        SYS_IPSendMsg2User(pstCcb, MSG_IP_CLOSEIND);//发送关闭指示到用户
//    }
    SYS_IPClearSelect(pstCcb);//清除资源
    pstCcb->ucState = SYS_IP_STATE_OCCUPY;

    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPConnect												
 函数的功能:     连接到对端								
 全局变量：																			
 输入参数：    pstCcb:          IP控制块
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPConnect(SYS_IP_CCB *pstCcb)
{
    //进行对端地址合法性判断
    if(( 0 == (pstCcb->stPeerAddr.ip >> 24))
        || (0 == (pstCcb->stPeerAddr.ip & 0x000000FF))
        || (0 == pstCcb->stPeerAddr.port))
    {
        return FAIL;
    }
    
     /*启动定时器*/
//    SYS_TimerStart(&pstCcb->usTimerId, IP_TASK_ID, pstCcb->uiTmConn,
//        IP_MSG_TM_CONNECT,(_USHORT)pstCcb->ucIndex, TIMER_MODE_NOLOOP);

    if (SUCC != SYS_SockOpen(SYS_IP_TYPE_TCP, 0, &pstCcb->sock))
    {
        //PRINT(MD(MOD_ID_IP,PL_ERR),"\n\rERR:SYS_IPConnect(),index=%d SockOpen", pstCcb->ucIndex);
        return FAIL;
    }
    if (SUCC != SYS_SockConnV4(pstCcb->sock, &pstCcb->stPeerAddr))
    {
        //PRINT(MD(MOD_ID_IP,PL_ERR),"\n\rERR:SYS_IPConnect(),index=%d SockConnV4", pstCcb->ucIndex);
        SYS_SockClose(pstCcb->sock);
        return FAIL;
    }
    
    SYS_IPFdSet(pstCcb->sock, &m_pTelnet->fdWrite);
    SYS_IPFdSet(pstCcb->sock, &m_pTelnet->fdException);
    pstCcb->ucState = SYS_IP_STATE_CONNECTTING;


    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPClose												
 函数的功能:   关闭CCB控制的SOCKET							
 全局变量：																			
 输入参数：    pstCcb:          IP控制块
                               iDir:               发起方向,0:用户发起,1:网络发起
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPClose(SYS_IP_CCB *pstCcb, _INT iDir)
{
    _UINT i;

    if(pstCcb->iType == SYS_IP_TYPE_UDP)
    {
	  if(pstCcb->ucState != SYS_IP_STATE_IDLE)
	  {
               SYS_IPClearSelect(pstCcb);//清除资源
               pstCcb->ucState = SYS_IP_STATE_IDLE;
	  }
         return SUCC;
    }
    
    switch(pstCcb->uiClass)
    {
    case SYS_IP_CLASS_SERVER://服务器端
        if (0 != iDir)//只有用户发起的才能关闭
        {
            return FAIL;
        }
        for (i = pstCcb->ucMinCcb; i <= pstCcb->ucMaxCcb; i++)
        {
            SYS_IPCloseSClient(&m_pTelnet->stCcb[i], 1);
            m_pTelnet->stCcb[i].ucState = SYS_IP_STATE_IDLE;
        }
        SYS_IPClearSelect(pstCcb);//清除资源
        pstCcb->ucState = SYS_IP_STATE_IDLE;
        break;

    case SYS_IP_CLASS_SERVERCLIENT://服务器端启动的客户连接
        SYS_IPCloseSClient(pstCcb, iDir);
        break;

    case SYS_IP_CLASS_CLIENT://主动连接别人的客户端
        if (SYS_IP_STATE_RUNNING == pstCcb->ucState)
        {
            /*SYS_IPSendMsg2User(pstCcb, MSG_IP_CLOSEIND);*///发送消息到上层
        }
        SYS_IPClearSelect(pstCcb);//清除资源
        if (0 == iDir)//用户发起的
        {
//            SYS_TimerStop(&pstCcb->usTimerId);
            pstCcb->ucState = SYS_IP_STATE_IDLE;
        }
        else//网络发起的
        {
            if (SYS_IP_STATE_CONNECTTING != pstCcb->ucState)//如果不是正在连接状态,正在连接状态由定时器发动连接
            {
//                SYS_TimerStop(&pstCcb->usTimerId);
                SYS_IPConnect(pstCcb);//连接到对端
            }
        }
        break;

    default:
        break;
    }

    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPDataReq												
 函数的功能:   处理数据请求						
 全局变量：																			
 输入参数：    pstMsg:          消息
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/

_INT SYS_IPDataReq(LONG_MESSAGE *pstMsg)
{
    SYS_IP_CCB          *pstCcb;
    _USHORT i,usRtnCnt,usMsgLen;

    if ((pstMsg->stHeader.usMsgLen == 0) || (pstMsg->stHeader.usMsgLen > MAX_IP_DATA_LEN) || (pstMsg->ucBuffer[0] >= MAX_IP_FSM))
    {
        //PRINT(MD(MOD_ID_IP,PL_ERR),"\n\rERR:SYS_IPDataReq(),MsgLen=%d index=%d", 
        //    pstMsg->stHeader.usMsgLen, pstMsg->ucBuffer[0]);
        return FAIL;
    }

    pstCcb = &m_pTelnet->stCcb[pstMsg->ucBuffer[0]];     /*消息ID对应呼叫控制块下标*/

    if (SYS_IP_STATE_RUNNING != pstCcb->ucState)//只有该状态能发送和接收数据
    {
        printf("\n\rERR:SYS_IPDataReq(),index=%d state=%d", pstCcb->ucIndex, pstCcb->ucState);
        //return FAIL;
    }

    if (SYS_IP_TYPE_TCP == pstCcb->iType)//TCP
    {
        if (pstCcb->stMyAddr.port == TELNET_PORT)
        {
            usRtnCnt = 0;
            for(i=0; i<pstMsg->stHeader.usMsgLen; i++)
            {
                sendTelnetBuf[usRtnCnt+i] = pstMsg->ucBuffer[1+i];
                 if (pstMsg->ucBuffer[1+i]=='\n') 
                 {
                    usRtnCnt++;                    
                    sendTelnetBuf[usRtnCnt+i] = '\r';                
                 }               
            }
  
            usMsgLen = pstMsg->stHeader.usMsgLen + usRtnCnt; 
            SYS_SockSend(pstCcb->sock, sendTelnetBuf, usMsgLen);
            
        }
        else
        {
            SYS_MEMCPY(sendTelnetBuf,&pstMsg->ucBuffer[1], (pstMsg->stHeader.usMsgLen));
            SYS_SockSend(pstCcb->sock, &pstMsg->ucBuffer[1], (pstMsg->stHeader.usMsgLen));
            SYS_MEMSET(pstMsg->ucBuffer, 0, MAX_IP_DATA_LEN);
        }
    }
    else//UDP或RAW SOCKET
    {
	SYS_SockUSend(pstCcb->sock, &pstMsg->ucBuffer[1], (pstMsg->stHeader.usMsgLen-1), &pstCcb->stPeerAddr);
    }

    return SUCC;
}

/**************************************************************
*
*
*
*
*
*
*
**************************************************************/
_INT TelnetIPConfig()
{
	_UCHAR ucBuffer[MAX_MSG_LEN];
	SYS_IP_CONFIG * cfg;
	LONG_MESSAGE *pstMsg = (LONG_MESSAGE *)ucBuffer;

	pstMsg->stHeader.ucDstTaskNo = IP_TASK_ID;
	pstMsg->stHeader.ucSrcTaskNo = CLI_TASK_ID;  
	pstMsg->stHeader.usMsgId = MSG_IP_CONFIG;
	pstMsg->stHeader.usMsgLen = sizeof(SYS_IP_CONFIG);

	cfg = (SYS_IP_CONFIG *)pstMsg->ucBuffer;

	cfg->ucCcbIndex = IP_FSM_CLI_SERVER;
	cfg->ucUser = CLI_TASK_ID;        
	cfg->iType = SYS_IP_TYPE_TCP;
	cfg->iProt = 6;
	cfg->stMyAddr.ip = htonl(INADDR_ANY);
	cfg->stMyAddr.port = TELNET_PORT;                             /*telnet 的默认端口号*/

	//cfg.stPeerAddr不必填
	cfg->stTCPCfg.uiClass       = SYS_IP_CLASS_SERVER;
	//cfg.stTCPCfg.uiTmConn不必填写
	cfg->stTCPCfg.ucMinCcb      = IP_FSM_CLI_FIRST_CLIENT;
	cfg->stTCPCfg.ucMaxCcb      = IP_FSM_CLI_SERVER - 1;
	cfg->stTCPCfg.ucClientPrio  = TCP_CLIENT_PRIO_SQUENCE;
			
	SYS_IPConfig(cfg);
	return SUCC;
}




/****************************************************************************************
 函数名称：    SYS_IPConfig												
 函数的功能:   配置IP部分					
 全局变量：																			
 输入参数：    pstCfg:          消息
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPConfig(SYS_IP_CONFIG *pstCfg)
{
   _UINT i;
    SYS_IP_CCB *pstCcb;

    if(pstCfg == NULL)
        return FAIL;

    if(pstCfg->ucCcbIndex >= MAX_IP_CCB_NUM)
    {
        //PRINT(MD(MOD_ID_IP,PL_ERR),"\n\rERR:SYS_IPConfig(),index=%d", pstCfg->ucCcbIndex);
        return FAIL;
    }

    pstCcb = &m_pTelnet->stCcb[pstCfg->ucCcbIndex];
        
    if (pstCcb->ucState != SYS_IP_STATE_IDLE)/*如果已经被占用*/
    {
        if((SYS_IP_TYPE_UDP==pstCcb->iType)&&(SYS_IP_TYPE_UDP==pstCfg->iType))
        {
	     pstCcb->iType = pstCfg->iType;
            pstCcb->iProt = pstCfg->iProt;
            pstCcb->ucUser = pstCfg->ucUser;
            //R3_SYS_ModIdGet(&pstCcb->stProvider.ucModId);
            //pstCcb->stProvider.usFsmId = (_USHORT)pstCfg->uiCcbIndex;
            pstCcb->stMyAddr = pstCfg->stMyAddr;
            pstCcb->stPeerAddr = pstCfg->stPeerAddr;
            pstCcb->ucProvider = IP_TASK_ID;
	
            return SUCC;
	  }
		
        if (SYS_IP_TYPE_TCP != pstCcb->iType)/*不是TCP*/
        {
	      //FD_CLR(pstCcb->sock, &m_pTelnet->stNmsIpData.fdRead);
	      //SYS_SockClose(pstCcb->sock);
            SYS_IPClose(pstCcb, 0);/*用户发起的*/
        }
        else
        {
            switch(pstCcb->uiClass)
            {
            case SYS_IP_CLASS_SERVER:/*服务器端*/
            case SYS_IP_CLASS_CLIENT:/*客户端*/
                SYS_IPClose(pstCcb, 0);
                break;
            case SYS_IP_CLASS_SERVERCLIENT:/*是本端连接产生的CCB*/
            default:
                return FAIL;
            }
        }
    }

    if (SYS_IP_TYPE_NOTUSE == pstCfg->iType)/*用户释放*/
    {
        return SUCC;
    }

    /*记录参数*/
    pstCcb->iType = pstCfg->iType;
    pstCcb->iProt = pstCfg->iProt;
    pstCcb->ucUser = pstCfg->ucUser;
    pstCcb->ucProvider = IP_TASK_ID;
    
    pstCcb->ucIndex = pstCfg->ucCcbIndex;
    pstCcb->stMyAddr = pstCfg->stMyAddr;
    pstCcb->stPeerAddr = pstCfg->stPeerAddr;

    if (pstCfg->iType != SYS_IP_TYPE_TCP)/*如果是UDP或RAW SOCKET*/
    {
        /*打开Socket*/
        if (SUCC != SYS_SockOpen(pstCfg->iType, pstCfg->iProt, &pstCcb->sock))
        {
	      //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),index=%d OpenSock", pstCcb->ucIndex);
            return FAIL;
        }

        pstCcb->ucState = SYS_IP_STATE_RUNNING;
        
        /*使用本端地址绑定Socket*/
        if(pstCcb->ucIndex > IP_FSM_CLI_FIRST_CLIENT)//IP_FSM_UP_BTS3
        {
            if(SUCC != SYS_SockBind(pstCcb->sock, &pstCcb->stMyAddr))
            {
                //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),index=%d BindSock", pstCcb->ucIndex);        
                return FAIL;
            }
        }

        SYS_IPFdSet(pstCcb->sock, &m_pTelnet->fdRead);
        
        return SUCC;
    }

    /*现在是TCP的了*/
    pstCcb->uiClass      = pstCfg->stTCPCfg.uiClass;
    pstCcb->uiTmConn     = pstCfg->stTCPCfg.uiTmConn;
    pstCcb->ucMinCcb     = pstCfg->stTCPCfg.ucMinCcb;
    pstCcb->ucMaxCcb     = pstCfg->stTCPCfg.ucMaxCcb;
    pstCcb->ucClientPrio = pstCfg->stTCPCfg.ucClientPrio;

    if (SYS_IP_CLASS_SERVER == pstCfg->stTCPCfg.uiClass)/*如果是服务器端*/
    {
        if (pstCfg->stTCPCfg.ucMinCcb >= MAX_IP_CCB_NUM || pstCfg->stTCPCfg.ucMaxCcb >= MAX_IP_CCB_NUM
            || pstCfg->stTCPCfg.ucMinCcb > pstCfg->stTCPCfg.ucMaxCcb
            || (pstCfg->stTCPCfg.ucMinCcb <= pstCfg->ucCcbIndex && pstCfg->ucCcbIndex <= pstCfg->stTCPCfg.ucMaxCcb))
        {
            //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),Server");
            return FAIL;
        }
        for (i = pstCfg->stTCPCfg.ucMinCcb; i <= pstCfg->stTCPCfg.ucMaxCcb; i++)
        {
            if ((m_pTelnet->stCcb[i].ucState != SYS_IP_STATE_IDLE) && (m_pTelnet->stCcb[i].ucState != SYS_IP_STATE_OCCUPY))
            {
                //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),index=%d state=%d", i, m_pTelnet->stCcb[i].ucState);
                return FAIL;
            }
        }

        if (SUCC != SYS_SockOpen(pstCfg->iType, pstCfg->iProt, &pstCcb->sock))
        {
            //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),index=%d SockOpen", pstCcb->ucIndex);
            return FAIL;
        }
        if (SUCC != SYS_SockBind(pstCcb->sock, &pstCfg->stMyAddr))
        {
            //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),index=%d SockBind", pstCcb->ucIndex);
            SYS_SockClose(pstCcb->sock);
            return FAIL;
        }
        if (SUCC != SYS_SockListen(pstCcb->sock))
        {
            //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConfig(),index=%d SockListen", pstCcb->ucIndex);
            SYS_SockClose(pstCcb->sock);
            return FAIL;
        }
        
        //printf("\n\rConfig Telnet sock :%x    ",pstCcb->sock);
        SYS_IPFdSet(pstCcb->sock, &m_pTelnet->fdRead);
        pstCcb->ucState = SYS_IP_STATE_LISTENED;

        /*初始化客户CCB*/
        for (i = pstCfg->stTCPCfg.ucMinCcb; i <= pstCfg->stTCPCfg.ucMaxCcb; i++)
        {
            m_pTelnet->stCcb[i].iType = pstCfg->iType;
            m_pTelnet->stCcb[i].iProt = pstCfg->iProt;
            m_pTelnet->stCcb[i].uiClass = SYS_IP_CLASS_SERVERCLIENT;
            m_pTelnet->stCcb[i].uiTmConn = pstCfg->stTCPCfg.uiTmConn;
            m_pTelnet->stCcb[i].ucMinCcb = pstCfg->stTCPCfg.ucMinCcb;
            m_pTelnet->stCcb[i].ucMaxCcb = pstCfg->stTCPCfg.ucMaxCcb;
            m_pTelnet->stCcb[i].ucUser = pstCfg->ucUser;
            m_pTelnet->stCcb[i].ucIndex = i;
            m_pTelnet->stCcb[i].ucProvider = IP_TASK_ID;
           
            m_pTelnet->stCcb[i].stMyAddr = pstCfg->stMyAddr;
            m_pTelnet->stCcb[i].stPeerAddr = pstCfg->stPeerAddr;
            m_pTelnet->stCcb[i].ucState = SYS_IP_STATE_OCCUPY;
        }
        //printf("\n\rConfig Telnet succ");
        return SUCC;
    }

    //如果是客户端
    SYS_IPConnect(pstCcb);

    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPAddrChange												
 函数的功能:   IP改变处理					
 全局变量：																			
 输入参数：    pstMsg:          消息
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPAddrChange(LONG_MESSAGE *pstMsg)
{
    _INT i;
    SYS_IP_CONFIG stCfg;
    SYS_IP_CCB *pstCcb;

    //首先全部关闭
    for (i = 0; i < MAX_IP_CCB_NUM; i++)
    {
        pstCcb = &m_pTelnet->stCcb[i];
        if (SYS_IP_STATE_IDLE == pstCcb->ucState)//如果是空闲状态
        {
            continue;
        }
        //关闭SOCKET
        SYS_IPClose(pstCcb, 0);
    }

    //清除TCP服务器端启动的客户连接
    for (i = 0; i < MAX_IP_CCB_NUM; i++)
    {
        pstCcb = &m_pTelnet->stCcb[i];
        if (SYS_IP_STATE_IDLE == pstCcb->ucState)//如果是空闲状态
        {
            continue;
        }
        //如果是TCP服务器端启动的客户连接，直接退出
        if (SYS_IP_TYPE_TCP == pstCcb->iType && SYS_IP_CLASS_SERVERCLIENT == pstCcb->uiClass)
        {
            pstCcb->ucState = SYS_IP_STATE_IDLE;
            continue;
        }
    }

    //重新配置
    for (i = 0; i < MAX_IP_CCB_NUM; i++)
    {
        pstCcb = &m_pTelnet->stCcb[i];
        if ((SYS_IP_TYPE_TCP == pstCcb->iType && SYS_IP_CLASS_SERVERCLIENT == pstCcb->uiClass)
            || (SYS_IP_TYPE_NOTUSE == pstCcb->iType))
            continue;

        //保存CCB中相关参数
        stCfg.ucCcbIndex = i;
        stCfg.iType = pstCcb->iType;
        stCfg.iProt = pstCcb->iProt;
        stCfg.ucUser = pstCcb->ucUser;
        stCfg.stMyAddr = pstCcb->stMyAddr;
        stCfg.stPeerAddr = pstCcb->stPeerAddr;
        stCfg.stTCPCfg.uiClass = pstCcb->uiClass;
        stCfg.stTCPCfg.uiTmConn = pstCcb->uiTmConn;
        stCfg.stTCPCfg.ucMinCcb = pstCcb->ucMinCcb;
        stCfg.stTCPCfg.ucMaxCcb = pstCcb->ucMaxCcb;
        if ((SYS_IP_TYPE_TCP == stCfg.iType&& SYS_IP_CLASS_SERVER == stCfg.stTCPCfg.uiClass)//自己是服务器端
            || SYS_IP_TYPE_UDP == stCfg.iType//自己是UDP
            || SYS_IP_TYPE_RAW == stCfg.iType)//自己是RAW SOCKET
        {
            /*待完成*/
            /*SYS_IPGetMyAddr(&stCfg.stMyAddr);*/
        }
        //重新配置CCB
        SYS_IPConfig(&stCfg);
    }

    return SUCC;
}

/****************************************************************************************
 函数名称：    SYS_IPTmProc												
 函数的功能:   定时器到期处理					
 全局变量：																			
 输入参数：    pstMsg:          消息
 输出参数:																				
 返回值:		       	
 调用此函数的函数列表：
*****************************************************************************************/
#if 0
_ULONG SYS_IPTmProc(TIMER_MSG *pMsg)
{
    
    SYS_IP_CCB *pstCcb;

    switch(pMsg->stMsgHeader.usMsgId)
    {
    case IP_MSG_TM_CONNECT://连接定时器超时
        if (pMsg->usPara >= MAX_IP_CCB_NUM)
            break;
        pstCcb = &m_pTelnet->stCcb[pMsg->usPara];
        if (pstCcb->ucState != SYS_IP_STATE_CONNECTTING)
            return FAIL;

//        pstCcb->usTimerId = NULL;
        SYS_IPClearSelect(pstCcb);//清除资源
        SYS_IPConnect(pstCcb);//连接到对端
        break;

    default:
        break;
    }
    
    return SUCC;
}
#endif
/****************************************************************************************
 函数名称：     SYS_IPMsgProc													
 函数的功能:    处理内部事件												
 全局变量：																			
 输入参数：     pstMsg:内部发送过来的消息,包括定时器和用户消息																		
 输出参数:																				
 返回值:		      SUCC or FAIL  																	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPMsgProc(LONG_MESSAGE* pstMsg)
{
    SYS_IP_CONFIG   *pstCfg;
    SYS_IP_CCB      *pstCcb;
    _UINT           i;
    SYS_IP_DATA     *pstIP;

    if(pstMsg == NULL)
        return FAIL;

    //时钟消息
//    if (TASK_NO_CLCK == pstMsg->stHeader.ucSrcTaskNo)     
//    {
//        SYS_CLCKMsgProc(&stSysData.stTaskInfo[IP_TASK_ID].stTimerMngt);
//        return SUCC;
//    }

    //PRINTMSG(MD(MOD_ID_IP,PL_LOW),(COMMON_HEADER *)pstMsg);

    switch(pstMsg->stHeader.usMsgId)
    {
    case MSG_IP_CONFIG:/*配置消息*/
        if (pstMsg->stHeader.usMsgLen != sizeof(pstCfg[0]))
        {
            //PRINT(MD(MOD_ID_IP,PL_ERR),"\n\rERR:SYS_IPMsgProc(),MsgLen=%d", pstMsg->stHeader.usMsgLen);
            break;
        }
        pstCfg = (SYS_IP_CONFIG*)pstMsg->ucBuffer;
        //PRINT(MD(MOD_ID_IP,PL_DEBUG),"\n\rDEBUG:[MP->IP],[MSG_IP_CONFIG],[index=%d]", pstCfg->ucCcbIndex);
        //PRINT(MD(MOD_ID_IP,PL_RUN),"\n\rRUN:SYS_IPMsgProc(),index=%d ip config",pstCfg->ucCcbIndex);
        SYS_IPConfig(pstCfg);
        break;
    case MSG_IP_ADDRCHANGE:/*IP地址发生改变*/
        //PRINT(MD(MOD_ID_IP,PL_RUN),"\n\rRUN:SYS_IPMsgProc(),AddrChange");
        SYS_IPAddrChange(pstMsg);
        break;
    case MSG_IP_CLOSEREQ:/*关闭请求*/
        if(pstMsg->ucBuffer[0] >= MAX_IP_CCB_NUM)
        {
            /*输出错误*/
            break;
        }
 
        pstCcb = &m_pTelnet->stCcb[pstMsg->ucBuffer[0]];
        //PRINT(MD(MOD_ID_IP,PL_DEBUG),"\n\rDEBUG:[MP->IP],[MSG_IP_CLOSEREQ],[index=%d]", pstMsg->ucBuffer[0]);
        //PRINT(MD(MOD_ID_IP,PL_RUN), "\n\rRUN:SYS_IPMsgProc(),index=%d ip close", pstMsg->ucBuffer[0]);
        SYS_IPClose(pstCcb, 0);
        break;
    case MSG_IP_DATAREQ://数据请求
        //PRINT(MD(MOD_ID_IP,PL_DEBUG),"\n\rDEBUG:[MP->IP],[MSG_IP_DATAREQ],[index=%d]",pstMsg->ucBuffer[0]);
        SYS_IPDataReq(pstMsg);
        break;
#if 0
    case MSG_EXIT:
        //PRINT(MD(MOD_ID_IP,PL_DEBUG),"\n\rDEBUG:[MP->IP],[MSG_EXIT]");
        pstIP = m_pTelnet;
        m_pTelnet = NULL;
     
        /*释放IP连接*/
        for (i = 0; i < MAX_IP_CCB_NUM; i++)
        {
            if (SYS_IP_STATE_IDLE == pstIP->stCcb[i].ucState
                || SYS_IP_STATE_OCCUPY == pstIP->stCcb[i].ucState)
            {
                continue;
            }

//            SYS_TimerStop(&pstIP->stCcb[i].usTimerId);
            
            SYS_SockClose(pstIP->stCcb[i].sock);
            /*SYS_IPSendMsg2User(&pstIP->stCcb[i], MSG_IP_CLOSEIND);*///发送消息到上层
        }

        SYS_FREE(pstIP);
        return FAIL;
#endif
    default:
        break;
    }

    return SUCC;
}

_INT SYS_SendToStdIn()
{
    _INT i;
    char* exitStr[] =
    {
        "exit",
        "quit",
        "bye"
    };

    if(HoldL2Flag==0)
    {
        write(CliPipeFd, ucParaBuf, ucParaLen);
    }
    else
    {
            
        for(i=0; i<SIZEOF(exitStr);i++ )
        {
            if(strncmp(ucParaBuf,exitStr[i], strlen(exitStr[i]))==0)
            {
                HoldL2Flag=0;
                RunShellFlag=0;
                RtnStdIO();
            }
        }
        SendCommandToL2(ucParaBuf, ucParaLen);
        taskDelay(2);
        //write(m_pTelnet->PipeFd, "L2->", 4);
    }
    
    SYS_MEMSET(ucParaBuf, 0, MAX_IP_DATA_LEN);
    ucParaLen = 0; 

    return SUCC;
    
}

/****************************************************************************************
 函数名称：     SYS_IPDataInd													
 函数的功能:    数据指示											
 全局变量：																			
 输入参数：     pstCcb: IP控制块																	
 输出参数:																				
 返回值:		      SUCC or FAIL  																	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPDataInd(SYS_IP_CCB *pstCcb)
{    
    _INT iLength,i;
    _INT iReturn,SendFlag;
    SYS_IP_ADDR addr;
    LONG_MESSAGE *pstMsg;
    
    SendFlag=0;
    pstMsg = (LONG_MESSAGE *)m_pTelnet->ucBuffer;

    //首先接收消息
    if (SYS_IP_TYPE_TCP == pstCcb->iType)
    {
        iLength = SYS_SockRecv(pstCcb->sock, pstMsg->ucBuffer, MAX_IP_DATA_LEN);
    }
    else
    {
        iLength = SYS_SockURecv(pstCcb->sock, pstMsg->ucBuffer, MAX_IP_DATA_LEN, &addr);
    }

    switch (pstCcb->iType)
    {
    case SYS_IP_TYPE_TCP:
        if (iLength <= 0)
        {
            if ((HoldL2Flag==1) && pstCcb->stMyAddr.port ==TELNET_PORT) 
            {
                HoldL2Flag=0;
                RunShellFlag=0;
                RtnStdIO();
                SendCommandToL2("bye", 3);
                taskDelay(10);
            }
            SYS_IPClose(pstCcb, 1);
        }
        else
        {
            
            if ((RunShellFlag==1) && pstCcb->stMyAddr.port ==TELNET_PORT) 
            {
                iReturn = write(m_pTelnet->PipeFd, (_UCHAR*)pstMsg->ucBuffer, iLength); //Echo
                SYS_MEMCPY(ucParaBuf+ucParaLen, (_UCHAR*)pstMsg->ucBuffer, iLength);
                ucParaLen += iLength;
                
                for(i=0; i< iLength; i++)
                {          
                    if ((pstMsg->ucBuffer[i] =='\n')||(pstMsg->ucBuffer[i] =='\r'))
                    {
                        SYS_SendToStdIn();                        
                        return SUCC;
                    }
                }
            }
            else
            {
                //发送消息到上层
                pstMsg->stHeader.usMsgLen = (_USHORT)iLength;
                SYS_IPSendMsg2User(pstCcb, MSG_IP_DATAIND);
            }
        }
        break;

    case SYS_IP_TYPE_UDP:
    case SYS_IP_TYPE_RAW:
        if (iLength > 0)
        {
            pstMsg->stHeader.usMsgLen = (_USHORT)iLength;
            SYS_IPSendMsg2User(pstCcb, MSG_IP_DATAIND);
        }
        else
        {
            int xxx = 0;
        }
        break;

    default:
        break;
    }

    return SUCC;
}

/****************************************************************************************
 函数名称：     SYS_IPConnectInd													
 函数的功能:    连接指示										
 全局变量：																			
 输入参数：     pstCcb: IP控制块																	
 输出参数:																				
 返回值:		      SUCC or FAIL  																	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPConnectInd(SYS_IP_CCB *pstCcb)
{
    /*_UINT i;*/
    SYS_SOCKET_ID sock;
    SYS_IP_CCB *pstCcb1;
    SYS_IP_ADDR stPeerAddr;
    _UCHAR ucIndex;

    if (SUCC != SYS_SockAccept(pstCcb->sock, &sock, &stPeerAddr))
    {
        //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_IPConnectInd(),index=%d SYS_SockAccept",pstCcb->ucIndex);
        return FAIL;
    }


    //if(GetIpFsmByIpaddr(&ucIndex, stPeerAddr) != SUCC)
    {
        /* 如果是Telnet客户端*/
	if(pstCcb->ucIndex == IP_FSM_CLI_SERVER)
	{
            if(m_pTelnet->stCcb[IP_FSM_CLI_FIRST_CLIENT].ucState != SYS_IP_STATE_OCCUPY)
            {
                //抢占式客户端
                //把第一个socket释放掉
                //printf("release old sock telnet \n");
                SYS_IPClearSelect(&m_pTelnet->stCcb[IP_FSM_CLI_FIRST_CLIENT]);
                //发送关闭指示给用户
                SYS_IPSendMsg2User(&m_pTelnet->stCcb[IP_FSM_CLI_FIRST_CLIENT], MSG_IP_CLOSEIND);
                //把释放了的socket连接让给当前新连接

            }
            ucIndex = IP_FSM_CLI_FIRST_CLIENT;
            m_pTelnet->stCcb[ucIndex].ucState =  SYS_IP_STATE_OCCUPY;

	}
	else
	{
		SYS_SockClose(sock);
		//PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nDBG:SYS_IPConnectInd(),Invalid  ip:%x port:%d close",stPeerAddr.ip,stPeerAddr.port);
		return SUCC;
	}
		
    }

    if((ucIndex > IP_FSM_CLI_SERVER) || (ucIndex < IP_FSM_CLI_FIRST_CLIENT))
    {
        printf("\r\nERR:SYS_IPConnectInd():ipFsm=%d!",ucIndex);
        return FAIL;
    }

    if(SYS_IP_STATE_OCCUPY != m_pTelnet->stCcb[ucIndex].ucState)
    {
	  printf("\r\nERR:SYS_IPConnectInd():IpFsm:%d:State:%d ", ucIndex, m_pTelnet->stCcb[ucIndex].ucState);
        return FAIL;
    }
    
    pstCcb1 = &m_pTelnet->stCcb[ucIndex];
    pstCcb1->sock = sock;
    CliSock = sock;
    pstCcb1->ucState = SYS_IP_STATE_RUNNING;

    SYS_IPFdSet(pstCcb1->sock, &m_pTelnet->fdRead);

    //发送连接指示消息到用户
    //printf("\n\rRUN:SYS_IPConnectInd(),index=%d", ucIndex);
    SYS_IPSendMsg2User(pstCcb1, MSG_IP_CONNECTIND);
    return SUCC;
}

_VOID showfd()
{
    _INT a=0,b=0;
    /*for(;;)
    {
        printf("this is a test, for telnet_task Suspend! \n");
        //taskDelay(1);
    }*/
    printf("please input the para\n");
    scanf("%d,%d", &a, &b);
    printf("\n\r  Telnet FD  0x%08x   sock%x a=%d b=%d ",m_pTelnet->maxFd, 
    m_pTelnet->stCcb[IP_FSM_CLI_SERVER].sock,a,b);
    /*printf("sendBuf:\n%s\n",sendTelnetBuf);
    printf("\nParaBuf:\n");
    printf("%s\n",ucParaBuf);
    */
    
}



/****************************************************************************************
 函数名称：     SYS_IPConnectingInd													
 函数的功能:    主动连接指示								
 全局变量：																			
 输入参数：     pstCcb:           IP控制块	
                                iFlag:              标志,SUCC:连接成功,FAIL:连接失败
 输出参数:																				
 返回值:		      SUCC or FAIL  																	
 调用此函数的函数列表：
*****************************************************************************************/
_INT SYS_IPConnectingInd(SYS_IP_CCB *pstCcb, _INT iFlag)
{
    FD_CLR(pstCcb->sock, &m_pTelnet->fdWrite);
    FD_CLR(pstCcb->sock, &m_pTelnet->fdException);
        

    if (FAIL == iFlag)//如果连接失败
    {
        printf("\r\nSYS_IPConnectingInd Socket Connect Fail\n");
        SYS_IPClose(pstCcb, 1);//网络发起的关闭,调用中会启动连接
        return SUCC;
    }
    //发送消息到上层用户
/*    SYS_PRINT(MD(FID_SYS, PL_RUN), "\r\nSocket Connect Succ. i = %d", pstCcb->ucProvider.usFsmId);
    SYS_TimerStop(&pstCcb->htTm);
    SYS_PRINT(MD(FID_IP, PL_RUN), "主动连接指示 = %d\r\n", pstCcb->ucProvider.usFsmId);*/
    /*SYS_IPSendMsg2User(pstCcb, MSG_IP_CONNECTIND);*/


    SYS_IPFdSet(pstCcb->sock, &m_pTelnet->fdRead);

    pstCcb->ucState = SYS_IP_STATE_RUNNING;

    return SUCC;
}

/****************************************************************************************
 函数名称：     SYS_IPTask													
 函数的功能:    接收IP监控消息						
 全局变量：																			
 输入参数：     
 输出参数:																				
 返回值:		      SUCC or FAIL  																	
 调用此函数的函数列表：
*****************************************************************************************/
_VOID SYS_IPTask()
{
    fd_set fdRead, fdWrite, fdException;    
    _INT iNum, iReadedNum, i, iLength;
    LONG_MESSAGE *pstMsg;
    SYS_IP_CCB *pClient;
    
    struct timeval timer = {TELNET_MAX_BLOCKED_TIME,0};    

    pstMsg = (LONG_MESSAGE *)m_pTelnet->ucBuffer;
    pClient =&m_pTelnet->stCcb[IP_FSM_CLI_FIRST_CLIENT];
    
    FOREVER
    {
        fdRead = m_pTelnet->fdRead;
        fdWrite = m_pTelnet->fdWrite;
        fdException = m_pTelnet->fdException;
        iReadedNum = 0;

        /*如果没有连接端口,只使用readfds*/
        if (pClient->ucState!=SYS_IP_STATE_RUNNING)
        {
            iNum = select(m_pTelnet->maxFd, &fdRead, &fdWrite, &fdException,0);
        }
        else
        {
            iNum = select(m_pTelnet->maxFd, &fdRead, &fdWrite, &fdException,&timer);
        }
        
        if (iNum==0 && RunShellFlag == 0)
        {
            SYS_SPRINTF(sendTelnetBuf,"\n\nTelnet Maintain Time Out, Quit Now!\n");
            SYS_SockSend(pClient->sock, sendTelnetBuf, SYS_STRLEN(sendTelnetBuf));
            //RtnStdIO();
            //taskDelay(100);
            SYS_IPClose(pClient,1);
        }
        

        /*如果是准备发给客户端的消息*/
        if(FD_ISSET(m_pTelnet->PipeFd, &fdRead))
        {
                iLength = read(m_pTelnet->PipeFd, pstMsg->ucBuffer+1, MAX_IP_DATA_LEN);
                if (iLength < 0)/*发生严重错误*/
                {
                    continue;                   
                }
                pstMsg->stHeader.ucDstTaskNo = IP_TASK_ID;
                pstMsg->stHeader.ucSrcTaskNo = CLI_TASK_ID;
                pstMsg->stHeader.usMsgId        = MSG_IP_DATAREQ;

                pstMsg->ucBuffer[0] = IP_FSM_CLI_FIRST_CLIENT;  //加上偏移量;
                pstMsg->stHeader.usMsgLen = iLength;                
                if (SYS_IPMsgProc(pstMsg) == FAIL)/*如果退出*/
                {
                    continue;
                }
           
            iReadedNum = 1;
        }
        if (iReadedNum >= iNum)
            continue;
        

        /*接收来自对端Socket 的消息*/
        for (i = 0; i < MAX_IP_FSM; i++)
        {
            //对于网络层,只在下面三种状态监视网络
            if (    m_pTelnet->stCcb[i].ucState != SYS_IP_STATE_LISTENED
                && m_pTelnet->stCcb[i].ucState != SYS_IP_STATE_CONNECTTING
                && m_pTelnet->stCcb[i].ucState != SYS_IP_STATE_RUNNING)
                continue;

            if (FD_ISSET(m_pTelnet->stCcb[i].sock, &fdRead))
            {
                switch(m_pTelnet->stCcb[i].ucState)
                {
                case SYS_IP_STATE_RUNNING:
                    SYS_IPDataInd(&m_pTelnet->stCcb[i]);
                    break;
                case SYS_IP_STATE_LISTENED:
                    //printf("New Telnet sock accept\n");
                    SYS_IPConnectInd(&m_pTelnet->stCcb[i]);
                    
                    break;
                default:
                    break;
                }

                iReadedNum++;
                if (iNum >= iReadedNum)
                    break;
                else
                    continue;
            }

            /*VXWORKS中异常描述符没有使用*/
            if (FD_ISSET(m_pTelnet->stCcb[i].sock, &fdWrite))
            {
                if (0 == SYS_SockIsConnected(m_pTelnet->stCcb[i].sock))
                    SYS_IPConnectingInd(&m_pTelnet->stCcb[i], SUCC);
                else
                    SYS_IPConnectingInd(&m_pTelnet->stCcb[i], FAIL);

                iReadedNum++;
                if (iNum >= iReadedNum)   break;
            }

        }
    }

}

/************************************************************************
* 函 数 名：SYS_MsgSend2IP
* 函数功能:发送消息到
* 输入参数:pstMsg:             消息
* 输出参数：
* 返 回 值：成功SUCC，失败FAIL
************************************************************************/
_INT SYS_MsgSend2IP(LONG_MESSAGE *pstMsg)
{
    _INT iReturn;
    
    if (NULL == m_pTelnet)
        return FAIL;

    if((pstMsg->stHeader.usMsgLen + SYS_MSG_HEAD_LEN) >= MAX_IP_DATA_LEN)
    {
        //PRINT(MD(MOD_ID_IP,PL_ERR),"\r\nERR:SYS_MsgSend2IP(),index=%d,MsgLen=%d", 
        //    pstMsg->ucBuffer[0],(pstMsg->stHeader.usMsgLen + SYS_MSG_HEAD_LEN));    
        return FAIL;
    }
    iReturn = write(m_pTelnet->PipeFd, (_UCHAR*)pstMsg + SYS_MSG_HEAD_LEN, (pstMsg->stHeader.usMsgLen)); 
    if(iReturn == ERROR)	
    {
        printf("\r\nERR:SYS_MsgSend2IP(),Pipe Write index=%d",pstMsg->ucBuffer[0]);	
        return FAIL;
    }

    return SUCC;
}

/************************************************************************
* 函 数 名：SYS_IPInitProc
* 函数功能:IP模块初始化
* 输入参数:
* 输出参数：
* 返 回 值：成功SUCC，失败FAIL
************************************************************************/
_INT SYS_IPInitProc()
{
    _INT i;

    m_pTelnet = (SYS_IP_DATA*)malloc(sizeof(m_pTelnet[0]));
    if (NULL == m_pTelnet)
    {
        return FAIL;
    }
    SYS_MEMSET(m_pTelnet, 0, sizeof(m_pTelnet[0]));
     
    SYS_SockIni();

    /* 创建管道队列 */
    if(pipeDevCreate("/stackPipe", 100, MAX_IP_DATA_LEN) != SUCC)
    {
        logMsg("SYS_IPInitProc(),stackPipe create failure!\n",0,0,0,0,0,0);
        return;
    }

    /* 打开管道*/
    m_pTelnet->PipeFd = open("/stackPipe", O_RDWR, 0);
    if(m_pTelnet->PipeFd == ERROR)
    {
        logMsg("SYS_IPInitProc(),Pipe open failure!\n",0,0,0,0,0,0);
        return;
    }
    //设置监视文件字
    FD_ZERO(&m_pTelnet->fdRead);
    FD_ZERO(&m_pTelnet->fdWrite);
    FD_ZERO(&m_pTelnet->fdException);
    SYS_IPFdSet(m_pTelnet->PipeFd, &m_pTelnet->fdRead);
    PipeFd = m_pTelnet->PipeFd;


    /* 创建管道队列 */
    if(pipeDevCreate("/telnetPipe", 100, MAX_IP_DATA_LEN) != SUCC)
    {
        logMsg("SYS_IPInitProc(), telnetPipe create failure!\n",0,0,0,0,0,0);
        return;
    }

    /* 打开Telnet 临时输入管道*/
    CliPipeFd = open("/telnetPipe", O_RDWR, 0);
    if(CliPipeFd == ERROR)
    {
        logMsg("SYS_IPInitProc(),telnetPipe open failure!\n",0,0,0,0,0,0);
        return;
    }

    return SUCC;
}
extern int g_blStartNewTelnet;
_VOID TelnetStart()
{
    if ((ERROR!=taskNameToId("tTelnetAcc")) ||(ERROR!=taskNameToId("tTelnetSvr")))
    {
        //已存在该任务
        printf("ERR:Telnet Started Already!\n");
        return;
    }

	g_blStartNewTelnet = 1;

    SYS_IPInitProc();
    CLI_IniProc();
    TelnetIPConfig();	/*配置TELNET*/
    taskDelay(100);
    SYS_TaskCreate("tTelnetAcc", 195, 20000, (FUNCPTR)SYS_IPTask, NULL,NULL);
    SYS_TaskCreate("tTelnetSvr", 205, 20000, (FUNCPTR)CLI_MsgTask, NULL,&TelnetSvr);
    printf("Telnet Started Succ!\n");
}
