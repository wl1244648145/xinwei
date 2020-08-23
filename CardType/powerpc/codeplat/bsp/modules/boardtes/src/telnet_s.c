#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <netpacket/packet.h>
#include <net/ethernet.h> 
#include <net/if.h>

unsigned int telnet_server_fd = 0;
int input_fd = 0;
int output_fd = 0;
int stderr_fd = 0;
int old_cfd = -1;

char telnet_server_starttime[30]="";;

void redup_std(int signum){
	dup2(input_fd, 0);
	dup2(output_fd, 1);
	dup2(stderr_fd, 2);
	if(old_cfd>2){
		close(old_cfd);
	}
	old_cfd = 0;
}

void signal_pipe(int signum){
	printf("recv signal: SIGPIPE\r\n");
	redup_std(SIGPIPE);
}

int isconnected(int sockfd){
	struct tcp_info info;
	int len = sizeof(info);
	getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);
	if(info.tcpi_state==TCP_ESTABLISHED){
		return 1;
	}
	return 0;
}

int bsp_get_ifflag(int index)
{
	int sock_fd, ret;
	struct ifreq ifreq;

	if (index < 0 || index > 3) {
		printf("error index %d\n", index);
		return -1;
	}

	sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (!sock_fd) {
		printf("Couldn't create socket\n");
		return -1;
	}

	sprintf(ifreq.ifr_name, "eth%d", index);

	ret = ioctl(sock_fd, SIOCGIFFLAGS, &ifreq);
	if (ret) {
		printf("Couldn't get info for if\n");
		perror("SIOCGIFFLAGS:");
		close(sock_fd);
		return -1;
	}

	close(sock_fd);

	if (ifreq.ifr_flags & IFF_RUNNING)
		return 1;
	else
		return 0;
}
void signal_handle(void)
{	
}

void *telnet_client_thread(void *arg){
	int cfd = (int)arg;
    union sigval tsval;
    struct sigaction act;
    act.sa_handler = signal_handle;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(50,&act,NULL);
	//close(0);
	//close(1);
	printf("welcom BBU1303 FrockTest Telnet Server! (server start time:%s)\r\np2041->", telnet_server_starttime);	
	fflush(stdout);
	sigqueue(getpid(),50,tsval);
	while(1){
		if(old_cfd!=cfd){
			break;
		}else if(isconnected(cfd)==0/*||bsp_get_ifflag(3)==0*/){
			redup_std(SIGPIPE);
			printf("telnet is disconnected!\r\np2041->");
			fflush(stdout);
			break;
		}
		sleep(1);
	}
	return NULL;
}

void *telnet_server_thread(void *arg){
	fd_set fd_sets;	
	struct sockaddr_in addr = {AF_INET};
	unsigned char recvbuf[1024*10]={0};
	unsigned int len = 0;
	int addrlen = sizeof(addr);	
	
	time_t t;
	struct tm *p;
	
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(9999);
	
	telnet_server_fd = socket(AF_INET, SOCK_STREAM,0);
	if(telnet_server_fd<0){
		perror("socket");
		return 0;
	}
	if(bind(telnet_server_fd, (struct sockaddr*)&addr, sizeof(addr))!=0){
		perror("bind");
		close(telnet_server_fd);
		return 0;
	}
	listen(telnet_server_fd, 10);
	
	time(&t);
	p = localtime(&t);
	sprintf(telnet_server_starttime, "%d/%02d/%02d %02d:%02d:%02d ", p->tm_year+1900,p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	printf("[%s]: frocktest: create telnet server....\r\n", telnet_server_starttime);
	while(1){
		struct sockaddr_in caddr = {AF_INET};
		int addr_len = sizeof(caddr);
		int client_fd = accept(telnet_server_fd, (struct sockaddr*)&caddr, &addrlen);
		printf("connected from:%s, client_fd=%d, old_cfd=%d\r\n", inet_ntoa(caddr.sin_addr), client_fd, old_cfd);
		
		if(client_fd > 0 ){
			pthread_t ctid;
			if(old_cfd > 2){
				close(old_cfd);
			}
			
			old_cfd = client_fd;
	
			if(dup2(client_fd, 0)<0){
				perror("dup2 0");
			}
			if(dup2(client_fd, 1)<0){
				perror("dup2 1");
			}
			if(dup2(client_fd, 2)<0){
				perror("dup2 2");
			}
			pthread_create(&ctid, NULL, telnet_client_thread, (void*)client_fd);
			pthread_detach(ctid);
		}
	}
	return 0;
}

int create_telnet_server(void){
	pthread_t tid;	
	input_fd = dup(0);
	output_fd = dup(1);
	stderr_fd = dup(2);
	signal(SIGPIPE, signal_pipe);
	pthread_create(&tid, NULL, telnet_server_thread, NULL);
	pthread_detach(tid);
}
