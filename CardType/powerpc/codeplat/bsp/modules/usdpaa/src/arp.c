#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>
#include <errno.h> 

// ---------------------
// ����ARP����ͷ�ļ�
#include<netinet/in.h>
#include<arpa/inet.h>
/* #include<linux/if_ether.h> */   
#include<ctype.h>
#include <fcntl.h>
// ---------------------

#define MAX_MAC_LEN 24  // MAC�ִ�����������󳤶�(byte)
#define COMPART_MAC ":"  // MAC�ִ��ķָ��, Warnning:�޸Ĵ˺꣬�������޸�Դ����!!!

#define MAX_BUF_LEN 128
static char strETH[MAX_BUF_LEN] = {0};  	// ����NIC����
static char strLocalMAC[MAX_BUF_LEN] = {0}; // ����MAC
static char strSrcIP[MAX_BUF_LEN] = {0}; 	// ����IP
char strDesMAC[MAX_BUF_LEN] = {0}; 	// Ŀ��MAC
static char strDesIP[MAX_BUF_LEN] = {0}; 	// Ŀ��IP

static char strTarIP[MAX_BUF_LEN] = {0};  /* Target IP address */
static char strTarMAC[MAX_BUF_LEN] = {0};  /* Target hardware address */
static int find_ip_flag = 0;	 /* 1 means find, 0 means not find */


static char * GetLocalMac(char *strEth); // get loacl NIC's MAC
static void set_ip_addr(char *,char *);   // ���IP
static void set_hw_addr(char buf[], char *str); // ���MAC
static char * GetMacByIP(char *strSrcIP, char *strSrcMAC, char *strDesIP ,char *strNIC); // ��ȡָ��IP��MAC
int arp_main( char *nic_name, char *nic_ip, char *des_ip);

/*
#define SRC_IP   "10.0.1.77"   // ԴIP
#define DES_IP   "10.0.1.35"   // Ŀ��IP
#define LOCAL_HW        "00:C0:4C:39:0D:6F" // 10.0.1.77��eth0��MAC
#define DEVICE          "eth0"    // �ӿ�
*/
#define PADD_MAC  "00:00:00:00:00:00" 	// ����MAC
#define DES_MAC   "FF:FF:FF:FF:FF:FF" 	// �㲥MAC
#define ARP_SEND_COUNT 3		// ����ARP�����ARP���ĸ���
#define RX_ARP_COUNT 3					// rx count of ARP response
#define SLEEP_MAX_US (1000 * 100)	/* unit microsecond */

struct ether_header
{
	unsigned char  ether_dhost[6];          /* destination eth addr */
	unsigned char  ether_shost[6];          /* source ether addr    */
	unsigned short ether_type;              /* packet type ID field */
};

struct arp_header
{
	unsigned short int ar_hrd;              /* Format of hardware address.  */
	unsigned short int ar_pro;              /* Format of protocol address.  */
	unsigned char ar_hln;                   /* Length of hardware address.  */
	unsigned char ar_pln;                   /* Length of protocol address.  */
	// -------------------------
	unsigned short int ar_op;               /* ARP opcode (command).  */
	unsigned char __ar_sha[6];              /* Sender hardware address.  */
	unsigned char __ar_sip[4];              /* Sender IP address.  */
	unsigned char __ar_tha[6];              /* Target hardware address.  */
	unsigned char __ar_tip[4];              /* Target IP address.  */
	// -------------------------
};

struct arp_packet
{
	struct ether_header ethhdr;
	struct arp_header arphdr;
	unsigned char padding[18];              /* filled with 0 */
};

/* arp reply:
*      op = 2
*      ethhdr.ether_dhost = arphdr.__ar_tha = switch hard addr
*      ethhdr.ether_shost = arphdr.__ar_sha = local hard addr
*      arphdr.__ar_tip = switch ip
*      arphdr.__ar_sip = victim ip
*/

#define FRAME_TYPE      0x0806                  /* arp=0x0806,rarp=0x8035 */
#define HARD_TYPE       1                       /* ethernet is 1 */
#define PROTO_TYPE      0x0800                  /* IP is 0x0800 */
#define OP_CODE         1                       /* arp=1/2,1Ϊ����2ΪӦ��,rarp=3/4 */

static int get_mac_by_ip(char *ethname, char *strDesIP);
static int cmd_ping(char *ethname, char *strDesIP);

static int cmd_ping(char *ethname, char *strDesIP)
{
    char cmd[128] = {0};
    int rv = -1;

    sprintf(cmd, "ping %s -I %s -c 1", strDesIP, ethname);
    if((rv = system(cmd)) < 0)
    {
        printf("excute cmd[%s] error, errno[%d].\n", cmd, rv);
        return rv;
    }

    return 0;
}

static int get_mac_by_ip(char *ethname, char *strDesIP)
{
    int rv = -1;
    FILE *fp = NULL;
    int num;
    int type, flags;
    char line[128] = {0};
    char ip[128] = {0};
    char hwa[128] = {0};
    char mask[128] = {0};
    char dev[128] = {0};
    int fg = 0;
    char *addr;
    int index = 0;
    static int ping = 0;

    if(ping == 0)
    {
        rv = cmd_ping(ethname, strDesIP);
        if(rv != 0)
        {
            printf("cmd ping error.\n");
            return rv;
        }
        ping = 1;
    }

    fp = fopen("/proc/net/arp", "r");
    if(fp == NULL)
    {
        printf("open file /proc/net/arp error.\n");
        return -1;
    }

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        mask[0] = '-'; mask[1] = '\0';
        dev[0] = '-'; dev[1] = '\0';
        /* All these strings can't overflow
         * because fgets above reads limited amount of data */
        num = sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
        			 ip, &type, &flags, hwa, mask, dev);
        if (num < 4)
            break;

        if((strcmp(ip, strDesIP) == 0) 
            && (strcmp(dev, ethname) == 0)
            && (strcmp(hwa, "00:00:00:00:00:00") != 0))
        {
            fg = 1;
            //printf("find mac addr [%s] of ip (%s).\n", hwa, strDesIP);
            break;
        }
    }

    if(fg == 0)
    {
        ping = 0;
    }
    else
    {
        addr = strtok(hwa,":");

        while(NULL != addr)
        {
            strDesMAC[index] = (unsigned char )strtol(addr, NULL, 16);
            addr = strtok(0, ":");
            index++;
        }        
    }
    
    fclose(fp);
    return 0;
}
void bsp_get_next_hop_mac(char *ethname,char *srcip,char *destip)
{
    //return arp_main("eth3","172.31.3.192", "172.31.3.89");
    #if 0
    arp_main(ethname,srcip, destip);
    #else 
    get_mac_by_ip(ethname, destip);
    #endif
    //printf("DesMAC = %s\n", strDesMAC);
}

// linux�»�ȡLAN��ָ��IP������MAC
// In: nic_name: ������������, nic_ip:����IP��des_ip:Ŀ��IP
// Out: 1��ʾ��ȡMAC�ɹ���des_ip�ѱ�ʹ��, -1��ʾ��������0��ʾdes_ipδʹ��

int arp_main( char *nic_name, char *nic_ip, char *des_ip)
{
    strcpy(strETH, nic_name); //"eth0";
    strcpy(strSrcIP, nic_ip); //"10.0.1.77";
    strcpy(strDesIP, des_ip); //"10.0.1.69";
    //printf("Run ......\n");
    //printf("��ȡ %s �ӿڵ�MAC ......\n", strETH);
    // ��ȡָ��NIC���Ƶ�MAC
    /*strLocalMAC = GetLocalMac(strETH);*/
    GetLocalMac(strETH);
#if 0
	wenxy_debug("Note: %s[%s]\n", strETH, strLocalMAC);
#endif
    // ��ȡָ���ӿ�MAC
    if ( 0 == strcmp( (const char*)strLocalMAC, (const char*)"") )
    {
        printf("Error: call strcmp() failed\n");
	printf("--------------------------------\n\n");
	return -1;
    }
    else
    {
        //printf("��ȡ�ӿ�MAC�ɹ�: %s [%s]\n", strETH, strLocalMAC);
    }
    // ��ȡָ��IP������MAC
    /*strDesMAC = GetMacByIP(strSrcIP, strLocalMAC, strDesIP, strETH);*/
    GetMacByIP(strSrcIP, strLocalMAC, strDesIP, strETH);
    //printf("DesMAC = %s\n", strDesMAC);
    if(1 == find_ip_flag)
    {
       // printf("Note:  DestIP: %s,  DestMAC: %s\n", strDesIP, strDesMAC);
	return 1;
    }
    else
    {
	printf("Note:  DestIP: %s does not use\n", strDesIP);
	return 0;
    }
}


// ��ȡ����ĳ������MAC
// In: strEth
// Out: ���ɹ�������MAC�ַ�����ʧ�ܣ�����""(�մ�)
static char * GetLocalMac(char *strEth)
{
	int s;
	struct ifreq buffer;
	char chBuff[MAX_MAC_LEN];
	unsigned int str_len = 0;
	unsigned int offset = 0;
	
	memset(chBuff, 0x0, sizeof(chBuff)); 
	s = socket(PF_INET, SOCK_DGRAM, 0); 
	if (-1 == s)
	{
		printf("Error: create socket failture\n");
		printf("--------------------------------\n\n");
		return "";
	}   
	memset(&buffer, 0x00, sizeof(buffer));    
	strcpy(buffer.ifr_name, strEth);  // "eth0"    
	if ( -1 == ioctl(s, SIOCGIFHWADDR, &buffer))
	{
		printf("Error: ��ȡ�ӿ� %s MAC ʧ��\n", strEth);
		printf("--------------------------------\n\n");
		return "";   
	}    
	close(s);

	offset = 0;
	for( s = 0; s < 6; s++ )
	{
		memset(chBuff, 0x0, sizeof(chBuff));
		sprintf( chBuff, "%.2X", (unsigned char)buffer.ifr_hwaddr.sa_data[s]);
		str_len = strlen(chBuff);
		memcpy(strLocalMAC + offset, chBuff, str_len);
		offset += str_len;
		//wenxy_debug("%.2X", (unsigned char)buffer.ifr_hwaddr.sa_data[s]);
		if (s < 5)
		{
			memset(chBuff, 0x0, sizeof(chBuff));
			sprintf( chBuff, "%s", COMPART_MAC);
			str_len = strlen(chBuff);
			memcpy(strLocalMAC + offset, chBuff, str_len);
			offset += str_len;
			//wenxy_debug(":");
		}
	}    
	//wenxy_debug("\n");
	return strLocalMAC; 
}


//-------------------------------------------------------
// ����ARP����������ARPӦ�����ȡ��MAC
// In: strSrcIP:����IP��strSrcMAC:����IP��MAC��strDesIP:������Ӧ��MAC��IP , strNIC:���ؽӿ���
// Out: ���ɹ�������MAC��ʧ�ܷ���""(�մ�)
static char * GetMacByIP(char *strSrcIP, char *strSrcMAC, char *strDesIP, char *strNIC)
{
	int sockfd;     // socket handle
	struct arp_packet arp;  // arp �����
	struct arp_packet arpRes; // arp Ӧ�ô��
	struct sockaddr sa;   // eth
	
	int iloop=0;
	char chSrcIP[24];
	char chDesIP[24];
	char chSrcMAC[24];
	char chNIC[8];
	unsigned int str_len = 0;
	unsigned int offset = 0;
	int i;
	int s;
	find_ip_flag = 0;
	
	memset(chSrcIP, 0x00, sizeof(chSrcIP));
	memset(chDesIP, 0x00, sizeof(chDesIP));
	memset(chSrcMAC, 0x00, sizeof(chSrcMAC));
	memset(chNIC, 0x00, sizeof(chNIC));
	sprintf(chSrcIP, "%s",  strSrcIP);
	sprintf(chDesIP, "%s",  strDesIP);
	sprintf(chSrcMAC, "%s", strSrcMAC);
	sprintf(chNIC, "%s", strNIC); 
#define SRC_IP   chSrcIP  // ԴIP
#define DES_IP   chDesIP  // Ŀ��IP
#define LOCAL_HW        chSrcMAC // eth0��MAC 
#define DEVICE   chNIC  // �����ӿ���
	
	memset(&arp, 0x00, sizeof(arp));
	memset(&arpRes, 0x00, sizeof(arpRes));
#if 0
	printf("ԴIP[%s]  ԴMAC[%s] Ŀ��IP[%s]\n", strSrcIP, strSrcMAC, strDesIP);  
#endif
	
	sockfd = socket(AF_INET, SOCK_PACKET, htons(0x0806));
	if(sockfd < 0)
	{
		printf("Error: create socket failed\n");
		printf("--------------------------------\n\n");   
		return "";
	}
	
	/*
	// ����socketΪ������ģʽ
	if ( -1 != fcntl(sockfd, F_SETFL, O_NONBLOCK) )
	{
	wenxy_debug("����socketΪ������ģʽ�ɹ�\n");
	}
	else
	{
	wenxy_debug("Warning: ����socketΪ������ģʽʧ��[errno = %d]\n", errno);
	}
	*/
	
	// ����socket���ճ�ʱ
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec= 100;
	if ( 0 == setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) )
	{
		//printf("����socket���ճ�ʱ�ɹ�\n"); 
	}
	else
	{
		printf("Warning: ����socket���ճ�ʱʧ��[errno = %d]\n", errno);
	}
#if 0
	printf("����socket SOCK_PACKET �ɹ�\n");
	printf("Note: ����ARP����� ......\n");
	printf("--------------------------------\n\n"); 
#endif
	
	// ����ARP����� 
	/* init arp packet header */
	arp.ethhdr.ether_type = htons(FRAME_TYPE);
	set_hw_addr( (char *)arp.ethhdr.ether_dhost, DES_MAC );  
	set_hw_addr( (char *)arp.ethhdr.ether_shost, LOCAL_HW );
#if 0
	printf("%x|", arp.ethhdr.ether_type);
	for (i = 0; i < 6; i++)
	{
		printf("%d_", arp.ethhdr.ether_dhost[i]);  
	}
	printf("|");
	for (i = 0; i < 6; i++)
	{
		printf("%d_", arp.ethhdr.ether_shost[i]);  
	} 
	printf("\n--------------------------------\n"); 
	printf("��ʼ��ARP��֡ͷ(��̫���ײ�)�ɹ�\n\n");  
#endif 
	
	/* init arp packet data */ 
	//printf("��ʼ��ARP��֡����(ARP�������) ......\n"); 
	//printf("--------------------------------\n"); 
	arp.arphdr.ar_hrd = htons(HARD_TYPE); // 1
	arp.arphdr.ar_pro = htons(PROTO_TYPE); // 0x0800
	arp.arphdr.ar_op = htons(OP_CODE);  // 1
	arp.arphdr.ar_hln = (unsigned char)(6);
	arp.arphdr.ar_pln = (unsigned char)(4);
#if 0
	printf("%d|%d|%d|%d|%d|\n", arp.arphdr.ar_hrd, arp.arphdr.ar_pro, 
		arp.arphdr.ar_op, arp.arphdr.ar_hln, arp.arphdr.ar_pln);
	printf("--------------------------------\n");    
#endif
	
	set_hw_addr((char *)arp.arphdr.__ar_tha, DES_MAC); // ����IP��MAC 
	set_hw_addr((char *)arp.arphdr.__ar_sha, LOCAL_HW); // �����ߵ�MAC 
	set_ip_addr((char *)arp.arphdr.__ar_tip, DES_IP); // ����MAC��IP 
	set_ip_addr((char *)arp.arphdr.__ar_sip, SRC_IP); // ԴIP
	bzero(arp.padding, 18); // ���18���ֽ�
#if 0
	for (i = 0; i < 6; i++)
	{
		printf("%d_", arp.arphdr.__ar_sha[i]);  
	}
	printf("|");
	for (i = 0; i < 6; i++)
	{
		printf("%d_", arp.arphdr.__ar_sip[i]);  
	}
	printf("|"); 
	for (i = 0; i < 6; i++)
	{
		printf("%d_", arp.arphdr.__ar_tha[i]);  
	}
	printf("|");
	
	for (i = 0; i < 6; i++)
	{
		printf("%d_", arp.arphdr.__ar_tip[i]);  
	}
	printf("|");
	printf("\n--------------------------------\n");  
#endif
	
	/* send arp reply packet */
	memset(&sa, 0x00, sizeof(sa));
	strcpy(sa.sa_data, DEVICE);
	
	// ����ARP��
	int nSendCount = ARP_SEND_COUNT;
	int nRecvByte = 0;
	while( (nSendCount --) > 0)
	{
		//printf("����ARP�����[%d Bytes]...... [��%d��]\n", sizeof(arp), nSendCount);   
		if( sendto(sockfd, &arp, sizeof(arp), 0, (struct sockaddr*) &sa, sizeof(sa)) < 0 )
		{
			printf("Error: ����ARP��ʧ�� [errno = %d]\n", errno);
			return "";
		}

		// ����ARPӦ���
		//printf("Note: ����ARPӦ�� ......\n");
		int nTryCount = RX_ARP_COUNT;
		int nAddrLen = sizeof(sa);	
		iloop = 0;
		do
		{
			/* because network and host delay */
			//usleep(SLEEP_MAX_US);
			iloop++;
			//printf("iloop->0x%lx\n",iloop);
			if (iloop > 1000)
			{
			    break;
			}
			nRecvByte = recvfrom(sockfd, &arpRes, sizeof(arpRes),0, (struct sockaddr*)&sa, (socklen_t*)&nAddrLen); 
		
			// ����������IP��ARPӦ������˳�while
			if ( nRecvByte >= 60 && 2 == ntohs(arpRes.arphdr.ar_op) )
			{
				char chBuff[MAX_MAC_LEN];   
				// ��ʽ��IP
				offset = 0;
				for (s = 0; s < 4; s++)
				{
					memset(chBuff, 0x00, sizeof(chBuff));
					sprintf( (char *)chBuff, "%d", (unsigned char)arpRes.arphdr.__ar_sip[s]);
					//wenxy_debug("chBuff: %s\n", chBuff);
					//strTarIP += chBuff;
					str_len = strlen(chBuff);
					memcpy(strTarIP + offset, chBuff, str_len);
					offset += str_len;
					if (s < 3)
					{
						memset(chBuff, 0x00, sizeof(chBuff));
						sprintf( (char *)chBuff, "%s", ".");
						//strTarIP += chBuff;
						str_len = 1;
						memcpy(strTarIP + offset, chBuff, str_len);
						offset += str_len;
					}
				}
				
				if ( !strcmp(strTarIP, strDesIP) )
				{
					//printf("\n����IP[%s] = Ӧ��IP[%s]\n", strDesIP, strTarIP);
					find_ip_flag = 1; /* find ip */
					goto analyse_arp_response;
				}
			}
		}while(1); // ����ARPӦ����Ĵ���
	}

	analyse_arp_response:
	//printf("�ѽ��յ�ARPӦ��� [%d Bytes]\n", nRecvByte);
	// ���ճ�ʱ�������
	if ( nRecvByte == -1 )
	{
		printf("Warning: ip->[%s] not reply\n", strDesIP);
		close(sockfd);
		return "";
	}
	
	//printf("����ARPӦ��� ......\n"); 
	char chBuff[MAX_MAC_LEN];   
	memset(chBuff, 0x00, sizeof(chBuff));
	// ��ʽ��IP
	offset = 0;
	for (s = 0; s < 4; s++)
	{
		memset(chBuff, 0x00, sizeof(chBuff));
		sprintf( (char *)chBuff, "%d", (unsigned char)arpRes.arphdr.__ar_sip[s]);
		//strTarIP += chBuff;
		str_len = strlen(chBuff);
		memcpy(strTarIP + offset, chBuff, str_len);
		offset += str_len;
		if (s < 3)
		{
			memset(chBuff, 0x00, sizeof(chBuff));
			sprintf( (char *)chBuff, "%s", ".");
			//strTarIP += chBuff;
			str_len = strlen(chBuff);
			memcpy(strTarIP + offset, chBuff, str_len);
			offset += str_len;
		}
	} 
	// ��ʽ��MAC
	memset(chBuff, 0x00, sizeof(chBuff));
	offset = 0;
	for (s = 0; s < 6; s++)
	{
		memset(chBuff, 0x00, sizeof(chBuff));
		sprintf( (char *)chBuff, "%c", (unsigned char)arpRes.arphdr.__ar_sha[s]);
		//strTarMAC += chBuff;
		//printf("%02x ",chBuff[s]);
		if (*(char *)chBuff == 0x00)
		{
		    str_len = 1;
		}
		else
		{
		    str_len = strlen(chBuff);
		}
		//printf("str_len:%x,chBuff:0x%02x\n",str_len,*(char *)chBuff);
            
		memcpy(strTarMAC + offset, chBuff, str_len);
		offset += str_len;
#if 0
        if (s < 5)
		{
			memset(chBuff, 0x00, sizeof(chBuff));
			sprintf( (char *)chBuff, "%s", COMPART_MAC);
			//strTarMAC += chBuff;
			str_len = strlen(chBuff);
			memcpy(strTarMAC + offset, chBuff, str_len);
			offset += str_len;
		}
#endif
	}
	// ���Ŀ��IP��Ŀ��MAC  
	//printf("Ӧ��IP[%s] ��Ӧ��MAC[%s]\n", strTarIP, strTarMAC);
	//printf("\n--------------------------------\n\n");
	close(sockfd); 
	// ���ر������MAC 
	//strcpy(strDesMAC, strTarMAC);
	memcpy(strDesMAC,strTarMAC,sizeof(strTarMAC));
	/* return */		
	return strDesMAC; 
}


// ���MAC  
static void set_hw_addr (char buf[], char *str)
{
	int i;
	char c, val;
	for(i = 0; i < 6; i++)
	{
		if (!(c = tolower(*str++)))
			perror("Invalid hardware address"),exit(1);
		if (isdigit(c))
			val = c - '0';
		else if (c >= 'a' && c <= 'f')
			val = c-'a'+10;
		else
			perror("Invalid hardware address"),exit(1);
		buf[i] = val << 4;
		if (!(c = tolower(*str++)))
			perror("Invalid hardware address"),exit(1);
		if (isdigit(c))
			val = c - '0';
		else if (c >= 'a' && c <= 'f')
			val = c-'a'+10;
		else
			perror("Invalid hardware address"),exit(1);
		buf[i] |= val;
		if (*str == ':')
			str++;
	}
}


// ���IP
static void set_ip_addr(char *buf, char *str)
{
	struct in_addr addr;
	memset(&addr, 0x00, sizeof(addr));
	addr.s_addr = inet_addr(str);  
	memcpy(buf, &addr, 4);
}

