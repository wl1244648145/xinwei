
#include "wbbu_util.h"
#include <arpLib.h>
#include <logLib.h>
#define closesocket close
#define assert(a) {if(!a) sys_abort();}
unsigned char l2_telnet_flag = 0;

void set_l2_telnet(unsigned char flag)
{
    l2_telnet_flag = flag;
}
/** Doubly-Linked Lists *****************************************************/
void ListInit(list_t *pList)
{
    pList->count = 0;
    pList->node.next = NIL;
    pList->node.previous = NIL;
}

void ListPushTail(list_t *pList, list_node_t *pNode)
{
    if( pList->node.next == NIL )
    {
        /** This is the first element of the list **/
        pList->node.next = pNode;
        pList->node.previous = pNode;
        pNode->next = NIL;
        pNode->previous = NIL;
        pList->count = 1;
    }
    else
    {
        pNode->next = NIL;
        pNode->previous = pList->node.previous;
        pList->node.previous->next = pNode;
        pList->node.previous = pNode;
        pList->count++;
    }
}

/** Add to the head of the list **/
void ListPushHead(list_t *pList, list_node_t *pNode)
{
    if( pList->node.next == NIL )
    {
        /** This is the first element of the list **/
        pList->node.next = pNode;
        pList->node.previous = pNode;
        pNode->next = NIL;
        pNode->previous = NIL;
        pList->count = 1;
    }
    else
    {
        pNode->previous = NIL;
        pNode->next = pList->node.next;
        pList->node.next->previous = pNode;
        pList->node.next = pNode;
        pList->count++;
    }
}

unsigned int ListCount(const list_t *pList)
{
    return(pList->count);
}

void ListDelete(list_t *pList, list_node_t *pNode)
{
    assert( pList->count );

    if(pNode->next == NIL)
    {
        /** Last in List **/
        pList->node.previous = pNode->previous;
    }
    else
    {
        pNode->next->previous = pNode->previous;
    }

    if(pNode->previous == NIL)
    {
        /** First in List **/
        pList->node.next = pNode->next;
    }
    else
    {
        pNode->previous->next = pNode->next;
    }

    pList->count--;
}

list_node_t *ListFirst(const list_t *pList)
{
    return(pList->node.next);
}

list_node_t *ListLast(const list_t *pList)
{
    return(pList->node.previous);
}

void ListInsertBefore(list_t *pList, list_node_t *pNext, list_node_t *pNode)
{
    if( pNext == NIL )
    {
        ListPushTail( pList, pNode);
        return;
    }
    else if( (pList->count == 0) || (pNext->previous == 0) )
    {
        ListPushHead( pList, pNode);
        return;
    }

    pNode->previous = pNext->previous;
    pNode->next = pNext;
    pNext->previous->next = pNode;
    pNext->previous = pNode;

    pList->count++;
}

list_node_t *ListNext(const list_node_t *pNode)
{
    return(pNode->next);
}


list_node_t *ListPrev(const list_node_t *pNode)
{
    return(pNode->previous);
}

list_node_t *ListPopHead(list_t *pList)
{
    list_node_t *N;

    N = pList->node.next;

    if (N)
    {
        ListDelete( pList, N );
    }
    return(N);
}

list_node_t *ListPopTail(list_t *pList)
{
    list_node_t *N;

    N = pList->node.previous;
    if (N)
    {
        ListDelete( pList, N );
    }
    return(N);
}

void    ListInsert(list_t *L, list_node_t *N, const void *(*keyof)(const list_node_t *), int (*compare)(const void *, const void *) )
{
    list_node_t *p = L->node.next;

    while( p && (compare( keyof(N), keyof(p) ) < 0) )
    {
        p = p->next;
    }

    if( p )
    {
        ListInsertBefore( L, p, N );
    }
    else
    {
        ListPushTail( L, N );
    }

}

list_node_t *ListFind(const list_t *pList, const void *pValue, const void *(*keyof)(const list_node_t *),
                      int (*compare)(const void *, const void *) )
{
    list_node_t *p;

    for( p = pList->node.next; p ; p = p->next )
    {
        if( !compare( pValue, keyof(p) ) )
        {
            return p;
        }
    }
    return 0;
}

//sock util
int InitSock()
{
#ifdef WIN32
    WSADATA wsaData;

    if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR)
    {
        fprintf(stderr,"WSAStartup failed with error %d\n",WSAGetLastError());
        WSACleanup();
        return -1;
    }
#endif
    return 0;
}

int EndSock()
{
#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}

static int connectTCP(u32_t ip,u_short port)
{
    struct sockaddr_in sin;
    int sfd,rv;

    sin.sin_family=AF_INET;
	sin.sin_port=htons(port);
	sin.sin_addr.s_addr=htonl(ip);
    memset(&(sin.sin_zero),0,8);

    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1)
    {
        printf("socket");
        return -1;
    }

    rv=connect(sfd,(struct sockaddr *)&sin,sizeof(struct sockaddr));
    if(rv==-1)
    {
        closesocket(sfd);
//        printf("connect");
        return -1;
    }
    return sfd;
}

static int passiveTCP(u16_t port)
{
    struct sockaddr_in loc_addr;
    int sfd;
    int rv;

    sfd=socket(AF_INET,SOCK_STREAM,0);

    if(sfd==-1)
    {
        printf("error: sfd==-1");
        return -1;
    }

    loc_addr.sin_family=AF_INET;
    loc_addr.sin_port=htons(port);
    loc_addr.sin_addr.s_addr=INADDR_ANY;
    memset(&(loc_addr.sin_zero),0,8);
    rv=bind(sfd,(struct sockaddr*)&loc_addr,sizeof(struct sockaddr));
    if(rv==-1)
    {
        printf("bind error: port %hd\n",port);
        closesocket(sfd);
        return -1;
    }
    rv=listen(sfd,5);
    if(rv==-1)
    {
        printf("listen");
        closesocket(sfd);
        return -1;
    }
    return sfd;

}

/*
para: ip and port should use net data fmt
ret: -1: fail, >0:sock fd
*/
static int passiveUDP(uint32 ip,uint16 port)
{
    int s;

    struct sockaddr_in sin;

    if((s=socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        return -1;
    }

    //sin.sin_len=sizeof(sin);
    sin.sin_family=AF_INET;
    sin.sin_port=port;
    sin.sin_addr.s_addr=ip;
    memset(&sin.sin_zero,0,8);

    /* bind the addr to socket */
    if( bind(s,(struct sockaddr *) &sin,sizeof(struct sockaddr))==-1)
    {
        printf("udp bind error: port %hd\n",port);
        return -1;
    }
    return s;
}

void sys_msleep(int ms);

int send_to_sck(int sck, void *data, int len) {
	int r,off=0,fail_cnt=0;
	char *pc;
	//
	pc=(char*)data;
	while(off<len){
		r=send(sck,pc+off,len-off,0);
		if(r<0)
			return -1;
		if(r==0) {
			sys_msleep(10);
			fail_cnt++;
			if(fail_cnt==300)
				return -1;
		}
		off+=r;
	}
	return off;
}

//sys op ifs

void *sys_malloc(int size) {
	return malloc(size);
}

void sys_free( void *p ) {
	free(p);
}

void sys_abort() {
	printf("sys_abort()...\n");
	while(1);
}

void sys_msleep(int ms) {
	int t=ms/10;
	if(t<=0)
		t=1;
	taskDelay(t);
}

#ifdef WIN32
static ULONG __stdcall threadEntry(void *lpv)
{
	thread_t thread=*(thread_t*)lpv;
	sys_free(lpv);
	thread.thread(thread.arg);
	return 0;
}
#endif

#ifdef DSP_BIOS
static int threadEntry(int lpv)
{
	thread_t thread=*(thread_t*)lpv;
	mem_free((void*)lpv);
	thread.thread(thread.arg);
	return 0;
}
#endif

sys_thread_t sys_thread_new(char *name, void (* thread)(void *arg), void *arg, 
	int stacksize, int prio)
{
#ifdef WIN32
	thread_t *pThread=(thread_t*)sys_malloc(sizeof(thread_t));
	ULONG thid;
	//
	pThread->thread=thread;
	pThread->arg=arg;
	CreateThread(0,0,threadEntry,pThread,0,&thid);
	return (sys_thread_t)thid;
#endif
#ifdef DSP_BIOS
	thread_t *pThread=mem_malloc(sizeof(thread_t));
	struct TSK_Attrs ta=TSK_ATTRS;
	//
	pThread->thread=thread;
	pThread->arg=arg;
	if(-1==prio)
		ta.priority=1;
	if(-1==stacksize) {
		ta.stacksize=2048;
	}
	return TSK_create(threadEntry,&ta,pThread);
//	return TSK_create(threadEntry,0,pThread);
#endif
#ifdef VXWORKS
	if(-1==prio)
		prio=250;
	if(-1==stacksize)
		stacksize=4096;
	taskSpawn(name,prio,0,stacksize,(FUNCPTR)thread,(int)arg,2,3,4,5,6,7,8,9,10);
#endif
}

//!sys op ifs

//my socket agent

struct nat_rec_t {
	char node[8];
	int type;//1:tcp, 2:udp, -1:END
	u16_t port;//agent port, net bytes seq
	u16_t port_map;
	u16_t src_port;
	u32_t ip;//agent ip, net bytes seq
	u32_t ip_map;
	u32_t src_ip;
};

struct sckag_sck_info_t {
	char node[8];
	int sck;
	int peer_sck;
	int sck_type;//1: tcp listen sck , 2: tcp sck, 3: udp sck
	int del_flag;
	nat_rec_t *nat;
};

struct my_sck_agent_env_t {
	list_t nat_table;
	list_t sck_table;
};

int g_sck_ag_debug=0;

void my_sck_agent_thread(void *arg) {
	my_sck_agent_env_t *env=(my_sck_agent_env_t*)sys_malloc(sizeof(my_sck_agent_env_t));
	int s, i;
	struct nat_rec_t *nat;
	fd_set rfds;
	list_node_t *N;
	struct timeval tv={1,0};
	list_t add_list;
	u8_t *buf;
	int bufsz=0x8000;
	//
	
	printf("my_sck_agent_thread()...\n");
	
	if(!arg)
		sys_abort();
	nat=(struct nat_rec_t*)arg;
	ListInit(&env->nat_table);
	ListInit(&env->sck_table);
	ListInit(&add_list);
	buf=new u8_t[bufsz];
	if(!buf)
		sys_abort();
	//
	for(i=0;;i++) {
		if(nat[i].type==1) {
			//tcp
			s=passiveTCP(ntohs(nat[i].port));
			if(-1==s)
				continue;
			sckag_sck_info_t *si=new sckag_sck_info_t;
			if(!si)
				sys_abort();
			si->sck=s;
			si->sck_type=1;
			si->del_flag=0;
			si->nat=&nat[i];
			ListPushTail(&env->sck_table,(list_node_t*)si);
		}
		else if(nat[i].type==2) {
			//udp
			s=passiveUDP(nat[i].ip,nat[i].port);
			if(-1==s)
				continue;
			sckag_sck_info_t *si=new sckag_sck_info_t;
			if(!si)
				sys_abort();
			si->sck=s;
			si->sck_type=3;//udp sck
			si->del_flag=0;
			si->nat=&nat[i];
			ListPushTail(&env->sck_table,(list_node_t*)si);
		}
		else if(nat[i].type==-1)
			break;
		ListPushTail(&env->nat_table,(list_node_t*)&nat[i]);
	}//for(i)

	while(1) {
		int max_sck=1, n;
		FD_ZERO(&rfds);
		for(N=ListFirst(&env->sck_table);N;N=ListNext(N)) {
			sckag_sck_info_t *si=(sckag_sck_info_t*)N;
			//
			FD_SET(si->sck,&rfds);
			if(si->sck>max_sck)
				max_sck=si->sck;
			if(2==si->sck_type) {
			FD_SET(si->peer_sck,&rfds);
			if(si->peer_sck>max_sck)
				max_sck=si->peer_sck;
			}
		}
		n=select(max_sck+1,&rfds,0,0,&tv);
		if(n<=0){
			continue;
		}
		for(N=ListFirst(&env->sck_table);N;N=ListNext(N)) {
			sckag_sck_info_t *si=(sckag_sck_info_t*)N;
			struct sockaddr_in addr;
			int alen, r;
			//
			if(FD_ISSET(si->sck,&rfds)) {
				if(1==si->sck_type) { //tcp listen sck
					int s1=-1;
					//
					alen=sizeof(addr);
					s=accept(si->sck,(struct sockaddr*)&addr,&alen);
					if(s<=0) 
						continue;
					printf("accept sck %d ip 0x%x port %hd\n",s,htonl(addr.sin_addr.s_addr),htons(addr.sin_port));
					nat=si->nat;
					if(nat) {
						s1=connectTCP(ntohl(nat->ip_map),ntohs(nat->port_map));
						if(s1!=-1) {
							si=new sckag_sck_info_t;
							si->sck=s;
							si->peer_sck=s1;
							si->sck_type=2;
							if(g_sck_ag_debug)
								printf("add tcp sck pair : sck %d peer_sck %d\n",s,s1);
							//store into a tmp list
							ListPushTail(&add_list,(list_node_t*)si);
						}
					}
					if(s1==-1){
						closesocket(s);
						if(g_sck_ag_debug)
							printf("s1==-1, close sck %d\n",s);
					}
				}
				else if(2==si->sck_type) { //tcp sck
					r=recv(si->sck,(char*)buf,bufsz,0);
					if(r>0){
						r=send_to_sck(si->peer_sck,buf,r);
						if(r<0){
							//to del later
							if(g_sck_ag_debug)
							{
								logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
							}
							si->del_flag=1;
							continue;
						}
					}
					else{
						if(g_sck_ag_debug)
							{
						logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
							}
						si->del_flag=1;
						continue;
					}
				}
				else if(3==si->sck_type) {//udp sck
					alen=sizeof(addr);
					r=recvfrom(si->sck,(char*)buf,bufsz,0,(struct sockaddr*)&addr,&alen);
					if(r<=0) {
						if(g_sck_ag_debug)
							{
						logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
							}
						si->del_flag=1;
						continue;
					}
					nat=si->nat;
					if(nat) {
						addr.sin_family=AF_INET;
						if(nat->ip_map==addr.sin_addr.s_addr && nat->port_map==addr.sin_port){
						addr.sin_addr.s_addr=nat->src_ip;
						addr.sin_port=nat->src_port;
						}
						else{
						nat->src_ip=addr.sin_addr.s_addr;//update src ip & port
						nat->src_port=addr.sin_port;
						addr.sin_addr.s_addr=nat->ip_map;
						addr.sin_port=nat->port_map;
						}
						alen=sizeof(addr);
						r=sendto(si->sck,(char*)buf,r,0,(struct sockaddr*)&addr,alen);
						if(r<=0) {
							if(g_sck_ag_debug)
							{
							logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
							}
							si->del_flag=1;
						}
					}
					else{
						if(g_sck_ag_debug)
						{
						logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
						}
						si->del_flag=1;
					}
				}
			}
			if(si->sck_type==2 && FD_ISSET(si->peer_sck,&rfds)) {
					r=recv(si->peer_sck,(char*)buf,bufsz,0);
					if(r>0){
						r=send_to_sck(si->sck,buf,r);
						if(r<0){
							//to del later
						   if(g_sck_ag_debug)
							{
							logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
						   	}
							si->del_flag=1;
							continue;
						}
					}
					else{
						if(g_sck_ag_debug)
							{
						logMsg("Ln%d: to del a si\n",__LINE__,2,3,4,5,6);
							}
						si->del_flag=1;
						continue;
					}
			}
		}//for(N)

		//add new sock info recs
		for(N=ListFirst(&add_list);N;N=ListFirst(&add_list)) {
			ListDelete(&add_list,N);			
			ListPushHead(&env->sck_table,N);
		}

		//latent delete part of sockets
		for(N=ListFirst(&env->sck_table);N;) {
			sckag_sck_info_t *si=(sckag_sck_info_t*)N;
			list_node_t *N1=ListNext(N);
			//
			if(1==si->del_flag) {
				ListDelete(&env->sck_table,N);
				if(2==si->sck_type) {
				if(-1!=si->sck) {
					if(g_sck_ag_debug)
							{
					printf("del a si: close sck %d\n",s);
						}
					closesocket(si->sck);
				}
				if(-1!=si->peer_sck) {
					if(g_sck_ag_debug)
							{
					printf("del a si: close peer sck %d\n",si->peer_sck);
						}
					closesocket(si->peer_sck);
				}
				}
				delete N;
			}
			N=N1;				
		}
	}
}

nat_rec_t *mySckAgConfig() {
	static nat_rec_t nat[10];
	int i=0, rc;
	//
	
	printf("mySckAgConfig()...\n");
	
	arpAdd ("10.0.0.2", "00:01:02:03:04:05", 0x4 );
	arpAdd ("10.0.0.3", "10:11:12:13:14:15", 0x4 );
	
//	rc=arpAdd ("172.16.24.25", "00:1c:25:ce:e1:77", 0x4 );
//	printf("rc %d\n",rc);
	
	//mytelnetd
	printf("add tcp lis port 334\n");
	nat[i].port=htons(334);
	nat[i].ip_map=inet_addr("10.0.0.2");
	nat[i].port_map=htons(333);
	nat[i].type=1;//tcp
	i++;
	
	//mytelnetd(C1)
	printf("add lis tcp port 334\n");
	nat[i].port=htons(335);
	nat[i].ip_map=inet_addr("10.0.0.3");
	nat[i].port_map=htons(333);
	nat[i].type=1;//tcp
	i++;

	//tcp echo srv
	nat[i].port=htons(77);
	nat[i].ip_map=inet_addr("10.0.0.2");
	nat[i].port_map=htons(7);
	nat[i].type=1;
	i++;

	//udp echo srv
	nat[i].port=htons(77);
	nat[i].ip_map=inet_addr("10.0.0.2");
	nat[i].port_map=htons(7);
	nat[i].type=2;
	i++;


	//tcp echo srv(C1)
	nat[i].port=htons(78);
	nat[i].ip_map=inet_addr("10.0.0.3");
	nat[i].port_map=htons(7);
	nat[i].type=1;
	i++;

	//udp echo srv(C1)
	nat[i].port=htons(78);
	nat[i].ip_map=inet_addr("10.0.0.3");
	nat[i].port_map=htons(7);
	nat[i].type=2;
	i++;

	nat[i].type=-1;
	return &nat[0];
}

extern "C"
void start_my_sck_agent_thread() {
	sys_thread_new("sckag",my_sck_agent_thread,mySckAgConfig(),-1,-1);
}

//!my socket agent

