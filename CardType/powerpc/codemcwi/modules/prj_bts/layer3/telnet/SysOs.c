/************************************************************************
*   ��Ȩ���У���������ͨ�Źɷ����޹�˾
*   �ļ����ƣ�SysOs.h
*   �ļ�����������ϵͳ��ʵ���ļ�
*                              ֧��32  λ����ϵͳallign 4 for struct;
*                              ����ϵͳͷ�ļ�
*				    ���¶��������������
*				    ���������������ź���������
*				    ��д�����ź�������Ϣ���к���
*   �޸ļ�¼��
*   1.  2005.11.08   JinWeimin   ����
*   2.  2006.07.24   liWenyan    �޸�
************************************************************************/



#include "SysOs.h"

/*****************************************************************************/

/*--------------------------------------------------------------------------------
//      ��������
//  ����:
//      p:                  ����
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//------------------------------------------------------------------------------*/
_INT SYS_TaskRoutine(VOID *p)
{
    SYS_TASK_CB cb;
    SYS_TASK_CB *cb1;
    
    if(NULL == p)
    {
        return(FAIL);
    }
    cb1 = (SYS_TASK_CB*)p;
    cb.f = cb1->f;
    cb.p = cb1->p;
    SYS_FREE(p);//��������Ժ��޸�Ϊ��̬�ڴ����
    cb.f(cb.p);
    return(SUCC);
}
/*--------------------------------------------------------------------------------
//      ����ϵͳ���������
//  ����:
//      p:                  ����
//  ����:
//      ��������ϵͳ��һ
//------------------------------------------------------------------------------*/
#ifdef UNIX
void* SYS_TaskProc(void* p)
{
    SYS_TaskRoutine(p);
    return 0;
}
#endif

#ifdef VXWORKS
int SYS_TaskProc(void* p)
{
    SYS_TaskRoutine(p);
    return 0;
}
#endif

#ifdef WINDOWS
void SYS_TaskProc(void *p)
{
    SYS_TaskRoutine(p);
}
#endif

/*--------------------------------------------------------------------------------
//      ��������
//  ����:
//      pTaskName:          ������
//      iTaskPri:           �������ȼ�
//      iTaskStackSize:     ��ջ��С
//      fTaskFun:           ��������
//      pPar:               ����
//  ����:
//      idTask:             �����ʶ
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//------------------------------------------------------------------------------*/
_INT SYS_TaskCreate(_CHAR *pTaskName, _INT iTaskPri, _INT iTaskStackSize, TASKFUNC fTaskFun, VOID *pPar, SYS_TASK_ID *idTask)
{
    SYS_TASK_CB* cb;
    
    cb = (SYS_TASK_CB*)SYS_MALLOC(FID_SYS, sizeof(SYS_TASK_CB));
    if(0 == cb)
    {
        return(FAIL);
    }
    cb->f = fTaskFun;
    cb->p = pPar;

#ifdef UNIX
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if(0 == pthread_create(idTask, &attr, SYS_TaskProc, cb))
        {
            return(SUCC);
        }
        return(FAIL);
    }
#endif

#ifdef WINDOWS
    {
        *idTask = _beginthread(SYS_TaskProc, iTaskStackSize, cb);
        return(NULL != (void *)*idTask ? SUCC : FAIL);
    }
#endif
    
#ifdef VXWORKS
    if (iTaskStackSize <= 0)
    {
        iTaskStackSize = 40960;//40k
    }
    *idTask = taskSpawn(pTaskName, iTaskPri, 0, iTaskStackSize, (FUNCPTR)SYS_TaskProc,
        (int)cb, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    return(ERROR != *idTask ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      ɾ������
//  ����:
//      task:               �����ʶ
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//------------------------------------------------------------------------------*/
_INT SYS_TaskRelease(SYS_TASK_ID task)
{
#ifdef UNIX
    return(0 == pthread_cancel(task) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    TerminateThread((HANDLE)task, 0);
    return(0 != CloseHandle((HANDLE)task) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(OK != taskDelete(task) ? SUCC : FAIL);
#endif
}
/*--------------------------------------------------------------------------------
//      ����˯��һ��ʱ��
//  ����:
//      ms:                 ʱ�䳤��(million second)
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//------------------------------------------------------------------------------*/
_INT SYS_Sleep(_UINT ms)
{
#ifdef UNIX
    struct timespec req;
    struct timespec rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&req, &rem);
    return(SUCC);
#endif

#ifdef WINDOWS
    Sleep(ms);
    return(SUCC);
#endif

#ifdef VXWORKS
    taskDelay(ms * sysClkRateGet() / 1000);
    return(SUCC);
#endif
}

/*--------------------------------------------------------------------------------
//      ��������ʼ��
//  ����:
//      mutex:              �ش��Ļ�����
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//------------------------------------------------------------------------------*/
_INT SYS_MutexCreate(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_init(mutex, USYNC_THREAD, NULL) ? SUCC : FAIL);/*�ڵ���������*/
#endif

#ifdef WINDOWS
    InitializeCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    *mutex = semMCreate(SEM_Q_PRIORITY |SEM_INVERSION_SAFE|SEM_DELETE_SAFE);/*���ȸ�һ���ź���*/
    return((NULL != *mutex) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      �������ͷ�
//  ����:
//      mutex:              ������
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//------------------------------------------------------------------------------*/
_INT SYS_MutexRelease(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_destroy(mutex) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    DeleteCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    return(OK == semDelete(*mutex) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      ����������
//  ����:
//      mutex:              ��
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//-------------------------------------------------------------------------------*/
_INT SYS_MutexLock(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_lock(mutex) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    EnterCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    return(ERROR != semTake(*mutex, WAIT_FOREVER) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//      ����������
//  ����:
//      mutex:              ��
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//-------------------------------------------------------------------------------*/
_INT SYS_MutexUnlock(SYS_MUTEX_ID* mutex)
{
#ifdef UNIX
    return(0 == mutex_unlock(mutex) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    LeaveCriticalSection(mutex);
    return(SUCC);
#endif

#ifdef VXWORKS
    return(ERROR != semGive(*mutex) ? SUCC : FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBCreate     
//  ����:
//      SYS_SEM_B_ID* semID;�������ź�����
//		SEM_B_STATE initialState;�������ź���״̬��
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//-------------------------------------------------------------------------------*/
_INT SYS_SemBCreate(SYS_SEM_B_ID* semID, SEM_B_STATE initialState)
{
#ifdef UNIX
	return(0 == sem_init(semID, USYNC_THREAD, initialState==SEM_FULL?1:0) ? SUCC : FAIL);/*�ڵ���������*/
#endif

#ifdef VXWORKS
    *semID = semBCreate(SEM_Q_FIFO, initialState);/*���ȸ�һ���ź���*/
    return((NULL != *semID) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	*semID=CreateSemaphore(NULL, initialState==SEM_FULL?1:0, 1, NULL);
	return ((*semID==INVALID_HANDLE_VALUE)? FAIL : SUCC);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBRelease     
//  ����:
//      SYS_SEM_B_ID* semID;�������ź�����
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//-------------------------------------------------------------------------------*/
_INT SYS_SemBRelease(SYS_SEM_B_ID* semID)
{
#ifdef UNIX
	return(0 == sem_destroy(mutex) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(OK == semDelete(*semID) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	return (TRUE==CloseHandle(*semID)?SUCC:FAIL);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBTake     
//  ����:
//      SYS_SEM_B_ID* semID;�������ź�����
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//-------------------------------------------------------------------------------*/
_INT SYS_SemBTake(SYS_SEM_B_ID* semID)
{
#ifdef UNIX
	return(0 == sem_wait(mutex) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(ERROR != semTake(*semID, WAIT_FOREVER) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	return (WAIT_FAILED==WaitForSingleObject(*semID,INFINITE)?FAIL:SUCC);
#endif
}

/*--------------------------------------------------------------------------------
//  SYS_SemBGive     
//  ����:
//      SYS_SEM_B_ID* semID;�������ź�����
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//-------------------------------------------------------------------------------*/
_INT SYS_SemBGive(SYS_SEM_B_ID* semID)
{
#ifdef UNIX
	return(0 == sem_post(mutex) ? SUCC : FAIL);
#endif

#ifdef VXWORKS
    return(ERROR != semGive(*semID) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
	return (TRUE==ReleaseSemaphore(*semID, 1, NULL)?SUCC:FAIL);
#endif
}


/*---------------------------------------------------------------------
 SYS_MsgQCreate - ��Ϣ���д���
 ����˵����
 maxMsgs            �� ���룬�����Ϣ��Ŀ
 maxMsgLength       �� ���룬�����Ϣ����
 msgq               �� �������Ϣ���к�
 ����ֵ: SUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
_INT SYS_MsgQCreate(_INT maxMsgs, _INT maxMsgLength, SYS_MSG_Q_ID *msgq)
{
    if (NULL == msgq)
    {
        return(FAIL);
    }

#ifdef UNIX
#endif

#ifdef VXWORKS
    *msgq = msgQCreate(maxMsgs, maxMsgLength, MSG_Q_FIFO);
    return((NULL != *msgq) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    if (SUCC != SYS_MutexCreate(&msgq->mutex))
        return FAIL;
    return((0 == _pipe((int*)&msgq->pipe, (maxMsgLength + sizeof(int)) * maxMsgs, _O_BINARY))
        ? SUCC : FAIL);
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQDelete - ɾ��һ����Ϣ����
 ����˵����
 msgq           �� ���룬��Ϣ���к�
 ����ֵ: SUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQDelete1(SYS_MSG_Q_ID *msgq)
#else
_INT SYS_MsgQDelete(SYS_MSG_Q_ID msgq)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return((OK == msgQDelete(msgq)) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    close(msgq->pipe.send);
    close(msgq->pipe.receive);
    SYS_MutexRelease(&msgq->mutex);
    return(SUCC);
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQSend - ����һ����Ϣ��ָ����Ϣ����
 ����˵����
 msgq           - ���룬��Ϣ���к�
 buffer         - ���룬��Ϣ����
 len            - ���룬��Ϣ����
 wait           - ���룬�ȴ���־
 prio           - ���룬���ȼ�
 ����ֵ: SUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQSend1(SYS_MSG_Q_ID *msgq, _VOID* buffer, _INT len, _INT wait, _INT prio)
#else
_INT SYS_MsgQSend(SYS_MSG_Q_ID msgq, _VOID* buffer, _INT len, _INT wait, _INT prio)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return((OK == msgQSend(msgq, (char*)buffer, (UINT)len, wait, prio)) ? SUCC : FAIL);
#endif

#ifdef WINDOWS
    if (SUCC != SYS_MutexLock(&msgq->mutex))
        return FAIL;
    if (-1 == write(msgq->pipe.send, &len, sizeof(len)))
    {
        SYS_MutexUnlock(&msgq->mutex);
        return(FAIL);
    }
    if (-1 == write(msgq->pipe.send, buffer, len))
    {
        SYS_MutexUnlock(&msgq->mutex);
        return(FAIL);
    }
    SYS_MutexUnlock(&msgq->mutex);
    return(SUCC);
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQRecv - ��ָ����Ϣ���н���һ����Ϣ
 ����˵����
 msgq           - ���룬��Ϣ���к�
 buffer         - ���룬��Ϣ���ݴ�Ż�����
 len            - ���룬��Ϣ����
 wait           - ���룬�ȴ���־
 ����ֵ: ���յ�����Ϣʵ�ʳ���
 -1:ʧ��
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQRecv1(SYS_MSG_Q_ID *msgq, _VOID* buffer, _INT len, _INT wait)
#else
_INT SYS_MsgQRecv(SYS_MSG_Q_ID msgq, _VOID* buffer, _INT len, _INT wait)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return (msgQReceive(msgq, (char*)buffer, (UINT)len, wait));
#endif

#ifdef WINDOWS
    _INT    iTmpLen, iBufLen, i;
    _UCHAR  *pBuf;

    if (sizeof(int) != read(msgq->pipe.receive, &iTmpLen, sizeof(int)))
    {
        return -1;
    }
    if (iTmpLen > len || 0 == iTmpLen)
    {
        return -1;
    }
/*    if (iTmpLen != read(msgq.receive, buffer, iTmpLen))
    {
        return(FAIL);
    }
    return(iTmpLen);*/
    iBufLen = 0;
    pBuf = (_UCHAR*)buffer;
    while(1)
    {
        i = read(msgq->pipe.receive, &pBuf[iBufLen], iTmpLen - iBufLen);//ע��������iTmpLen - iBufLen���㣬�����������
        if (i <= 0)//0:����,-1:������
        {
            return -1;//����ֱ�ӷ���0
        }
        iBufLen += i;
        if (iBufLen >= iTmpLen)//�������
            break;
    }
    return iBufLen;
#endif
}

/*---------------------------------------------------------------------
 SYS_MsgQNumMsgs - ��ȡָ����Ϣ���е�������Ϣ��Ŀ
 ����˵����
 msgq           - ���룬��Ϣ���к�
 ����ֵ: ��Ϣ���е�������Ϣ��Ŀ
------------------------------------------------------------------------*/
#ifdef WINDOWS
_INT SYS_MsgQNumMsgs1(SYS_MSG_Q_ID *msgq)
#else
_INT SYS_MsgQNumMsgs1(SYS_MSG_Q_ID msgq)
#endif
{
#ifdef UNIX
#endif

#ifdef VXWORKS
    return(msgQNumMsgs(msgq));
#endif

#ifdef WINDOWS
    return 0;
#endif
}

//SOCKET
//--------------------------------------------------------------------------------
//      SOCKETϵͳ��ʼ��
//  ����:
//      ��
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockIni()
{
#ifdef UNIX
    return(SUCC);
#endif
    
#ifdef WINDOWS
    WSADATA ws;
    return(0 == WSAStartup(MAKEWORD(2, 1), &ws) ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    return(SUCC);
#endif
}
//--------------------------------------------------------------------------------
//      SOCKETϵͳ����
//  ����:
//      ��
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockDes()
{
#ifdef UNIX
    return(SUCC);
#endif
    
#ifdef WINDOWS
    return (0 == WSACleanup() ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    return(SUCC);
#endif
}
//--------------------------------------------------------------------------------
//      ��ȡ������
//  ����:
//      ��
//  ����:
//      1��             û�����ݿɶ�
//      2��             SOCKET�Ѿ��رջ���������
//--------------------------------------------------------------------------------
_INT SYS_SockGetError()
{
#ifdef UNIX
    return 1;
#endif
    
#ifdef WINDOWS
    return ((WSAGetLastError() == WSAEWOULDBLOCK) ? 1 : 2);
#endif

#ifdef VXWORKS
    return 1; /* errnoGet();*/
#endif
}
//--------------------------------------------------------------------------------
//      ��һ��SOCKET
//  ����:
//      type:               ��ʽ(1:TCP 2:UDP 3:RAW)
//      protocol:           Э���
//  ����:
//      sock:               SOCKET���
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockOpen(_INT type, _INT iProtocol, SYS_SOCKET_ID *sock)
{
    int optval;

    if (SYS_IP_TYPE_RAW == type)//RAW SOCKET
    {
        sock[0] = socket(AF_INET, SOCK_RAW, iProtocol);
#ifdef WINDOWS
        //��WINDOWS��ʹ��RAW SOCKET,�����ȴ���һ��,���򽫲����յ����ݰ�
        if (-1 != sock[0])
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = 0;
            addr.sin_addr.s_addr = 0;
            bind(sock[0], (struct sockaddr*)&addr, sizeof(addr));
        }
#endif
    }
    else
    {
        sock[0] = socket(AF_INET, (SYS_IP_TYPE_TCP == type) ? SOCK_STREAM : SOCK_DGRAM, 0);
    }
    if (-1 == sock[0])
        return(FAIL);
    optval = 1;
    if (1 == type)//�����TCP
    {
        //setsockopt(sock[0], SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof (optval));
        setsockopt (sock[0], IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof (optval));
    
    }
    //����Ϊ�����õ�,64K����,��������ʽ
    setsockopt(sock[0], SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    optval = 65536;//64k
    setsockopt(sock[0], SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval));
    setsockopt(sock[0], SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
    
    optval = 200;//128k
    SYS_SockSetBlockMode(sock[0], 1);//����Ϊ��������ʽ
    return(SUCC);
}
//--------------------------------------------------------------------------------
//      ����һ��SOCKET��������ʽ
//  ����:
//          sock:           SOCKET���
//          mode:           0:����,1:������
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockSetBlockMode(SYS_SOCKET_ID sock, _INT mode)
{
#ifdef UNIX
    int flag = mode;
    return (-1 != ioctl(sock, FIONBIO, (int)&flag) ? SUCC : FAIL);
#endif
    
#ifdef WINDOWS
    long flag = mode;
    return (0 == ioctlsocket(sock, FIONBIO, &flag) ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    int flag = mode;
    return (ERROR != ioctl(sock, FIONBIO, (int)&flag) ? SUCC : FAIL);
#endif
}
//--------------------------------------------------------------------------------
//      ��IPV4��ַת����Ϊ�ڲ���ַ
//  ����:
//      ip��                IP��ַ�� "10.0.10.28"
//  ����:
//      addr                IPV4��ַ
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockGetV4Addr(_CHAR *cIP, _ULONG *addr)
{
    *addr = inet_addr(cIP);
    return(SUCC);
}
//--------------------------------------------------------------------------------
//      �󶨱��ض˿�
//  ����:
//      sock��              SOCKET���
//      addr:               ��ַ
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockBind(SYS_SOCKET_ID sock, SYS_IP_ADDR *addr)
{
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    return (0 == bind(sock, (struct sockaddr*)&s, sizeof(s)) ? SUCC : FAIL);
}
//--------------------------------------------------------------------------------
//      ����һ��SOCKET
//  ����:
//      sock��              SOCKET���
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockListen(SYS_SOCKET_ID sock)
{
#ifdef UNIX
    return (0 == listen(sock, 8) ? SUCC : FAIL);//LISTENQ
#endif
    
#ifdef WINDOWS
    return (0 == listen(sock, 8) ? SUCC : FAIL);
#endif
    
#ifdef VXWORKS
    return (OK == listen(sock, 8) ? SUCC : FAIL);
#endif
}
//--------------------------------------------------------------------------------
//      ����һ��SOCKET
//  ����:
//      sock��              SOCKET���
//  ����:
//      sock1:              ���ӵ�SOCKET���
//      SUCC:               �ɹ�
//      FAIL:               ʧ��

// modify by liwei 2006.7.31 ���ӶԶ�ip��ַ�ķ���,TCP������Ҫ���ݶԶ�Ip����IP FSM
//--------------------------------------------------------------------------------
_INT SYS_SockAccept(SYS_SOCKET_ID sock, SYS_SOCKET_ID* sock1, SYS_IP_ADDR *stPeerAddr)
/*_INT SYS_SockAccept(SYS_SOCKET_ID sock, SYS_SOCKET_ID* sock1)*/
{   
    struct sockaddr_in s;
    int len;
  
    if(stPeerAddr == NULL)
        return FAIL;    
    
    memset(&s, 0, sizeof(s));
    len = sizeof(s);
  

    //���ﲻ���Է�IP��ַ���ݵ��û�������ȫ������ȫ���ϲ�Э����ƣ��봫���޹�
    *sock1 = accept(sock, (struct sockaddr*)&s, &len);
    if (sock1[0] != -1)
    {
        SYS_SockSetBlockMode(sock1[0], 1);//����Ϊ��������ʽ
        stPeerAddr->ip = s.sin_addr.s_addr;
        stPeerAddr->port = SYS_NTOHS(s.sin_port);
    }
    return ((*sock1 != -1) ? SUCC : FAIL);
}
//--------------------------------------------------------------------------------
//      ���ӵ�һ��IP+PORT
//  ����:
//      sock��              SOCKET���
//      addr:               �Զ˵�ַ
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockConnV4(SYS_SOCKET_ID sock, SYS_IP_ADDR* addr)
{
#ifdef UNIX
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    connect(sock, (struct sockaddr*)&s, sizeof(s));
    return(SUCC);
    //��connect����,��ʱ����-1,����errno����ΪEINPROGRESS,�⼴connect�Ծ�  
    //�ڽ��л�û�����
#endif
    
#ifdef WINDOWS
    struct sockaddr_in s;
    int lasterror;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    if (0 == connect(sock, (struct sockaddr*)&s, sizeof(s)))
        return(SUCC);
    lasterror = WSAGetLastError();
    if (WSAEWOULDBLOCK == lasterror || WSAEINPROGRESS == lasterror)
        return(SUCC);
    return(FAIL);
#endif
    
#ifdef VXWORKS
    struct sockaddr_in s;

    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr->port);
    s.sin_addr.s_addr = addr->ip;
    connect(sock, (struct sockaddr*)&s, sizeof(s));
    return(SUCC);
    //��connect����,��ʱ����-1,����errno����ΪEINPROGRESS,�⼴connect�Ծ�  
    //�ڽ��л�û�����
#endif
}
//--------------------------------------------------------------------------------
//      ����TCP����
//  ����:
//      sock��              SOCKET���
//      buf��               ������
//      size��              ��������С
//  ����:
//      _INT��              �������ݳ���
//--------------------------------------------------------------------------------
_INT SYS_SockRecv(SYS_SOCKET_ID sock, _UCHAR* buf, _INT size)
{
    _INT i;
    i = recv(sock, (char*)buf, size, 0);

#ifdef UNIX
    if (0 == i || -1 == i)
        return 0;
#endif

#ifdef WINDOWS
    if (0 == i || SOCKET_ERROR == i)
        return 0;
#endif

#ifdef VXWORKS
    if (0 == i || ERROR == i)
        return 0;
#endif

    return i;
}
//--------------------------------------------------------------------------------
//      ����TCP����
//  ����:
//      sock��              SOCKET���
//      buf��               ������
//      size��              ��������С
//  ����:
//      _INT��              �������ݳ���
//--------------------------------------------------------------------------------
_INT SYS_SockSend(SYS_SOCKET_ID sock, _UCHAR* buf, _INT len)
{
    return (send(sock, (char*)buf, len, 0));
}
//--------------------------------------------------------------------------------
//      ����UDP���ݰ�
//  ����:
//      sock��              SOCKET���
//      buf��               ������
//      len��               ���ݴ�С
//      addr��              ��ַ
//  ����:
//      _INT��              �������ݳ���
//--------------------------------------------------------------------------------
_INT SYS_SockURecv(SYS_SOCKET_ID sock, _UCHAR* buf, _INT size, SYS_IP_ADDR* addr)
{
#ifdef UNIX
    struct sockaddr_in s;
    int i;
    _INT j;

    i = sizeof(s);
    j = recvfrom(sock, (char*)buf, size, 0, (struct sockaddr*)&s, &i);
    if (0 == j || -1 == j)//????????
        return 0;
    if(0 != addr)
    {
        addr[0].ip = s.sin_addr.s_addr;
        addr[0].port = s.sin_port;
    }
    return j;
#endif
    
#ifdef WINDOWS
    struct sockaddr_in s;
    int i, j;

    i = sizeof(s);
    j = recvfrom(sock, buf, size, 0, (struct sockaddr*)&s, &i);
    if (0 == j || SOCKET_ERROR == j)
        return 0;
    if(NULL != addr)
    {
        addr[0].ip = s.sin_addr.S_un.S_addr;
        addr[0].port = s.sin_port;
    }
    return j;
#endif
    
#ifdef VXWORKS
    struct sockaddr_in s;
    int i, j;

    i = sizeof(s);
    j = recvfrom(sock, (_CHAR *)buf, size, 0, (struct sockaddr*)&s, &i);
    if (0 == j || ERROR == j)
        return 0;
    if(NULL != addr)
    {
        addr[0].ip = s.sin_addr.s_addr;
        addr[0].port = s.sin_port;
    }
    return j;
#endif
}
//--------------------------------------------------------------------------------
//      ����UDP���ݰ�
//  ����:
//      sock��              SOCKET���
//      buf��               ������
//      len��               ���ݴ�С
//      addr��              ��ַ
//  ����:
//      _INT��              �������ݳ���
//--------------------------------------------------------------------------------
_INT SYS_SockUSend(SYS_SOCKET_ID sock, _UCHAR* buf, _INT len, SYS_IP_ADDR* addr)
{
#ifdef UNIX
    struct sockaddr_in s;
    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr[0].port);
    s.sin_addr.s_addr = addr[0].ip;
    return (sendto(sock, (char*)buf, len, 0, (struct sockaddr*)&s, sizeof(s)));
#endif
    
#ifdef WINDOWS
    struct sockaddr_in s;
    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = htons(addr[0].port);
    s.sin_addr.s_addr = addr[0].ip;
    return (sendto(sock, buf, len, 0, (struct sockaddr*)&s, sizeof(s)));
#endif

#ifdef VXWORKS
    struct sockaddr_in s;
    memset(&s,0,sizeof(s));
    s.sin_family=AF_INET;
    s.sin_port=htons(addr[0].port);
    s.sin_addr.s_addr = addr[0].ip;
    return (sendto(sock, (_CHAR *)buf, len, 0, (struct sockaddr*)&s, sizeof(s)));
#endif
}
//--------------------------------------------------------------------------------
//      �ر�һ��SOCKET���
//  ����:
//      sock��              SOCKET���
//          
//  ����:
//      SUCC:               �ɹ�
//      FAIL:               ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockClose(SYS_SOCKET_ID sock)
{
#ifdef UNIX
//    if(-1 == unlink((char*)&sock)) return -1;
//    shutdown(sock, 2);//#define SD_BOTH         0x02
    return (0 == close(sock) ? 0 : -1);
#endif
    
#ifdef WINDOWS
    shutdown(sock, 2);//#define SD_BOTH         0x02
    return (0 == closesocket(sock) ? 0 : -1);
#endif
    
#ifdef VXWORKS
    shutdown(sock, 2);//#define SD_BOTH         0x02
    return (ERROR != close(sock) ? 0 : -1);
#endif
}
//--------------------------------------------------------------------------------
//      �ж�һ��SOCKET�Ƿ�������״̬
//  ����:
//      sock��              SOCKET���
//  ����:
//      0:                  �ɹ�
//      -1:                 ʧ��
//--------------------------------------------------------------------------------
_INT SYS_SockIsConnected(SYS_SOCKET_ID sock)
{
    struct sockaddr_in s;
    int i;

    i = sizeof(s);
#ifdef UNIX
    return (0 == getpeername(sock, (struct sockaddr*)&s, &i) ? 0 : -1);
#endif

#ifdef WINDOWS
    return (0 == getpeername(sock, (struct sockaddr*)&s, &i) ? 0 : -1);
#endif

#ifdef VXWORKS
    return (OK == getpeername(sock, (struct sockaddr*)&s, &i) ? 0 : -1);
#endif

}

_VOID SYS_PRINT(const _UCHAR * szDbgStr)
{
#ifdef VXWORKS
	/*logMsg("%s", (_INT)szDbgStr, 0, 0, 0, 0, 0);*/
	printf("%s", szDbgStr);
#endif
	
#ifdef WINDOWS
	printf("%s", szDbgStr);
#endif
	
#ifdef UNIX
	printf("%s", szDbgStr);
#endif
}
