
//#include <winsock.h>
#include <vxWorks.h>
#include <sockLib.h>
#include <inetLib.h>
#include <hostLib.h>
#include <ioLib.h>
#include <selectLib.h> 
#include <stdio.h>
#include <stdlib.h>

#if 1
//basic data types
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef char int8;
typedef short int16;
typedef int int32;
//typedef unsigned int u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int u32_t;
typedef unsigned short u16_t;
typedef unsigned char u8_t;
typedef short int s16_t;
typedef signed char s8_t;
typedef signed int s32_t;
#endif//1

#ifdef __cplusplus
extern "C" {
#endif

/*dual list*/
#define	NIL	NULL

typedef struct list_node_t {
	struct list_node_t *next;
	struct list_node_t *previous;
}list_node_t;

typedef struct{
	list_node_t node;
	unsigned int count;

	unsigned int busy;
} list_t;

typedef const void *(*KEYOF_FUNC_LIST)(const list_node_t *);
typedef int (*CMP_FUNC_LIST)(const void *, const void *);

/** Doubly Linked List ******************************************************/
void	ListInit(list_t *pList);

unsigned int	
	ListCount(const list_t *pList);
list_node_t	*ListFirst(const list_t *pList);
list_node_t	*ListLast(const list_t *pList);
list_node_t	*ListNext(const list_node_t *pNode);
list_node_t	*ListPrev(const list_node_t *pNode);

#define	ListEmpty(L)	((L)->count == 0)
void	ListInsertAfter(list_t *pList, list_node_t *pPrev, list_node_t *pNode);
void	ListInsertBefore(list_t *pList, list_node_t *pNext, list_node_t *pNode);
void	ListDelete(list_t *pList, list_node_t *pNode);

void	ListPushHead(list_t *pList, list_node_t *pNode);
void	ListPushTail(list_t *pList, list_node_t *pNode);
list_node_t	*ListPopHead(list_t *pList);
list_node_t	*ListPopTail(list_t *pList);

void	ListInsert(list_t *, list_node_t *, const void *(*)(const list_node_t *), int (*)(const void *, const void *) );
void	ListWalk(const list_t *, void (*)(list_node_t *, void *), void * );
list_node_t	*ListFind(const list_t *, const void *, const void *(*)(const list_node_t *), int (*)(const void *, const void *) );
void	ListSort(list_t *, const void *(*)(const list_node_t *), int (*)(const void *, const void *) );

void	ListConcat(list_t *pDstList, list_t *pAddList);
void	ListExtract(list_t *pSrcList, list_node_t *pStartNode, list_node_t *pEndNode, list_t *pDstList);

list_node_t	*ListNth(const list_t *pList, unsigned int nodenum);
list_node_t	*ListStep(list_node_t *pNode, int nStep);
int	ListIndex(const list_t *pList, list_node_t *pNode);

//sock util
//int passiveTCP(u16_t port);
//int passiveUDP(uint32 ip,uint16 port);
//int connectTCP(u32_t ip,u_short port);
int connectUDP(char *server,short port);
int InitSock();
int EndSock();
int udp_send(int usock,u_long host,short port,char *pack,int plen);
int udp_recv(int usock,uint32 *host,uint16 *port,char *pack,int plen);
//!sock util

//tcp msg util

typedef struct {
	u32_t len;//data len
	u16_t type;
	u16_t magic;
} tcp_msg_hdr_t;

#define TCP_MSG_HDR_SZ sizeof(tcp_msg_hdr_t)

#define TCP_RBUF_SIZE 0x20000 //128K

typedef struct {
	u32_t start_pos;
	u32_t end_pos;
	char rbuf[TCP_RBUF_SIZE];
	tcp_msg_hdr_t mh;
} tcp_comm_buf_t;

u16_t chksum16(void *p, int len);
void writeTcpMsgHdr(void *pc, u32_t len, u16_t type);
void tcpCommBufInit(tcp_comm_buf_t *cb);
int tcpCommBufRoom(tcp_comm_buf_t *cb);
void *tcpCommBufEndPos(tcp_comm_buf_t *cb,int *buf_len);
int addTcpData(tcp_comm_buf_t *cb, u8_t *data, int len);
int recvTcpMsg(tcp_comm_buf_t *cb, void **data, tcp_msg_hdr_t *mh);
int sendTcpMsgToSck(int sck, int len, int type, void *msg);
int send_to_sck(int sck, void *data, int len);
//tcp msg util END

//sys call
typedef void *sys_sem_t;
void *sys_malloc(int sz);
void sys_free(void *);
sys_sem_t sys_sem_new(u8_t count);
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout);
void sys_sem_signal(sys_sem_t sem);
void sys_sem_free(sys_sem_t sem);
void sys_assert();
void sys_abort();
typedef u32_t sys_thread_t;
sys_thread_t sys_thread_new(char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio);
void sys_msleep(u32_t t);
u32_t sys_now();
//!sys call

//my socket agent
void start_my_sck_agent_thread();

#ifdef __cplusplus
}
#endif
