
//net card packet capture task

#include "pcap.h"
extern "C"
{
#include "remote-ext.h"
}
#include "pcap-bpf.h"
//#include "sockstorage.h"
#include "packet32.h"
#include "ntddndis.h"

#include "Object.h"
#include "MsgQueue.h"
#include "Message.h"
#include "taskDef.h"
#include "LogArea.h"
#include "Timer.h"

//#include "aplib.h"

#include "WinPCAP.h"
#include "L3DataLib.h"
#include "L3DataCPESM.h"

static int pcap_test_flag=0;
int g_bts_card_id=2;

int CPEcardIp[3] = {0};

u_char g_broadcast_mac_addr[6];
T_MacRecordTable *g_mac_table;

int get_mac_addr(char *name,char *outMacAdd);

/************************ mac addr table class **********************/

static unsigned long mac_record_t_hash(const void *v1)
{
    short i,j,*q=(short *)v1;
    //
    j=0;
    for(i=0;i<3;i++)
        j+=q[i];

    //Trace(1,"\nhash = %d",j);

    return j;
}

static int mac_record_t_compare(const void *v1, const void *v2)
{
    int r;
    //
    r=memcmp(v1,v2,6);
    if(r>0)
        r=1;
    if(r<0)
        r=-1;
    if(r)
        return r;
    return 0;
}

static const void * mac_record_t_keyof(const struct hash_node_t *v1)
{
    mac_record_t *p=(mac_record_t *)v1;
    return p->mac_addr;
}

T_MacRecordTable::T_MacRecordTable()
{
    HashInit(&htab,T_MacRecordTable_HEntryNum,mac_record_t_hash,mac_record_t_compare,mac_record_t_keyof,hentry);
}

bool T_MacRecordTable::Insert(mac_record_t *prec)
{
    mac_record_t *p1;
    //

#ifdef USE_TRACE

    {
        char str[200];
        BinToHex((u_char*)prec->mac_addr,6,(u_char*)str);
        Trace(1,"\nT_MacRecordTable::Insert(): %s , %d",str,prec->card_id);
    }
#endif

    if(prec->card_id>20)
        abort();

    p1=Find(prec->mac_addr);
    if(p1)
    {
#ifdef USE_TRACE
        Trace(1,"\nmac addr already exist !");
#endif
        abort();
        return false;
    }

    HashInsert(&htab,(hash_node_t *)prec);

    return true;
}

void T_MacRecordTable::Delete(mac_record_t *prec)
{
    HashDelete(&htab,(hash_node_t *)prec);
}

mac_record_t *T_MacRecordTable::Find(const UINT8 *addr)
{
    mac_record_t *p;
    //

#ifdef USE_TRACE

    {
        BinToHex((u_char*)addr,6,(u_char*)str);
        Trace(1,"\nT_MacRecordTable::Find(): %s count %d",
              str,htab.count);
    }
#endif

    p=(mac_record_t *)HashFind(&htab,addr);

#ifdef USE_TRACE

    if(p)
    {
        Trace(1,"\nLn103: p {%s %d} ",str,p->card_id);
    }
#endif

    return p;
}

/************************ net card packet capture task ***********************/

//LPADAPTER g_adapter_arr[Max_Num_Adapter];
pcap_t *g_adapter_arr[Max_Num_Adapter];
u_char mac_addr_arr[Max_Num_Adapter][6];
int AdapterNum=0;
//g_bts_card_id=1;
//static char AdapterList[Max_Num_Adapter][400];

pcap_t * get_adapter(int card_no)
{
    return g_adapter_arr[card_no];
}

int get_adpter_num()
{
    return AdapterNum;
}

u_char *get_net_card_mac_addr(int card_no)
{
    return mac_addr_arr[card_no];
}

pcap_if_t       *alldevs;

int init_pcap_vars()
{
    pcap_if_t *d;
    char errbuf[PCAP_ERRBUF_SIZE];
    int i;
    pcap_t *pcap;

    //
	memset(g_broadcast_mac_addr,0xff,6);
	g_mac_table = new T_MacRecordTable;
    errbuf[0]=0;

    /* Retrieve the device list on the local machine */
    pcap_findalldevs(&alldevs, errbuf);
    printf("\nLn203: %s",errbuf);

    /* Print the list */
    for(i=0,d=alldevs; d; d=d->next,i++)
    {
        if (d->name && d->description)
            printf("\n%d) %s : %s", i,d->name,d->description);

        pcap = pcap_open( d->name,        // name of the device
                          65536,            // portion of the packet to capture
                          // 65536 guarantees that the whole packet will be captured on all the link layers
                          PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
                          -1,             // read timeout
                          NULL,             // authentication on the remote machine
                          errbuf            // error buffer
                        );

        printf("\nLn229: pcap : 0x%x",pcap);
        printf("\nLn230: errbuf : %s",errbuf);

		if ( 0 != d->addresses )
		{
			CPEcardIp[i] = *( (int*)&(d->addresses->addr->sa_data[2]) );
			struct in_addr IpAddress;
			IpAddress.S_un.S_addr= CPEcardIp[i] ;
            printf("\nLn231: Ip-Address : %s\n",inet_ntoa(IpAddress));
		}

        g_adapter_arr[i]=pcap;

        get_mac_addr(d->name,(char*)mac_addr_arr[i]);
    }

    AdapterNum=i;

    printf("\r\n-----------------------------------");
    printf("\r\n-----------------------------------");
    printf("\r\nPlease Select an adapter for BTS.");
	printf("\ng_bts_card_id:");
    scanf("%d",&g_bts_card_id);
    printf("\r\n...OK...g_bts_card_id = %d\r\n", g_bts_card_id);
	pcap_test_flag=0;
    return 0;
}

int get_mac_addr(char *name,char *outMacAdd)
{
    LPADAPTER   lpAdapter = 0;
    DWORD       dwErrorCode;
    PPACKET_OID_DATA  OidData;
    BOOLEAN     Status;

    lpAdapter =   PacketOpenAdapter(name);

    if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
    {
        dwErrorCode=GetLastError();
        printf("Unable to open the adapter, Error Code : %lx\n",dwErrorCode);

        return -1;
    }

    //
    // Allocate a buffer to get the MAC adress
    //

    OidData = (struct _PACKET_OID_DATA *)malloc(6 + sizeof(PACKET_OID_DATA));
    if (OidData == NULL)
    {
        printf("error allocating memory!\n");
        PacketCloseAdapter(lpAdapter);
        return -1;
    }

    //
    // Retrieve the adapter MAC querying the NIC driver
    //

    OidData->Oid = OID_802_3_CURRENT_ADDRESS;

    OidData->Length = 6;
    ZeroMemory(OidData->Data, 6);

    Status = PacketRequest(lpAdapter, FALSE, OidData);
    if(Status)
    {
        printf("The MAC address of the adapter is %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
               (PCHAR)(OidData->Data)[0],
               (PCHAR)(OidData->Data)[1],
               (PCHAR)(OidData->Data)[2],
               (PCHAR)(OidData->Data)[3],
               (PCHAR)(OidData->Data)[4],
               (PCHAR)(OidData->Data)[5]);
    }
    else
    {
        printf("error retrieving the MAC address of the adapter!\n");
    }
    memcpy(outMacAdd,(OidData->Data),6);

    free(OidData);
    PacketCloseAdapter(lpAdapter);
    return (0);
}


/****************************
*pkt_data发送到网卡card_id上
*/
void PutToCard(int card_id, int len, UINT8 *pkt_data)
{
	pcap_t *pcap=get_adapter(card_id);
	if(pcap)
        pcap_sendpacket(pcap, pkt_data, len);
    return;
}


void PutToMacSendQueue_cpe(int card_id,int len,UINT8 *pkt_data)
{

#ifdef M_TGT_L3
    CTaskCPESM::SendToUTDM( len, pkt_data );
	//PutToCard(card_id, len, pkt_data);
#else
    //PutToCard(card_id, len, pkt_data);
	PutToCard(g_bts_card_id, len, pkt_data);
#endif
}


void ProcessMacPacket_cpe(int card_id, pcap_pkthdr *header,u_char *pkt_data)
{
    mac_record_t *prec;
    u_char *p;
	char str[100];

    if(header->caplen!=header->len)
    {
        printf("\ncpe: header->caplen!=header->len");
        return;
    }

    p=pkt_data;

#if 1

    {
        BinToHex((u_char*)p,20,(u_char*)str);
#ifdef USE_TRACE
        Trace(1,"\nCPE#%d : %s",card_id,str);
#endif
    }
#endif

    //p+6 is source mac addr
    prec=g_mac_table->Find((const unsigned char *)p+6);
    if(prec)
    {
#ifdef USE_TRACE
        Trace(2,"\nLn109 :  cpe#%d : prec->card_id %d",card_id,prec->card_id);
#endif
        if(prec->card_id==g_bts_card_id)
        {
            //this packet comes from bts , so discard it
#ifdef USE_TRACE
            Trace(2,"\ncpe#%d discard a packet ...",card_id);
#endif
        }
        else
        {
            //user has changed the net port, so update src mac record
            //prec->card_id is the old net if used by the cpe with mac addr p+6
            if(prec->card_id!=card_id)
            {
                BinToHex((u_char *)p+6,6,(u_char *)str);
#ifdef USE_TRACE
                Trace(3,"\nsrc mac %s change from %d to %d",
                      str,prec->card_id, card_id);
#endif
                prec->card_id = card_id;
            }
            else
            {
#ifdef USE_TRACE
                if(memcmp(p,g_broadcast_mac_addr,6)==0)
                {
                    Trace(2,"\nsend broadcast to bts , case 1 ");
                }
                else
                {
                    Trace(1,"\nLn122: send to bts ...");
                }
#endif
                PutToMacSendQueue_cpe(card_id,header->len,pkt_data);
            }
        }
    }
    else
    {

        //process broadcast packet
        if(memcmp(p,g_broadcast_mac_addr,6)==0)
        {
#ifdef USE_TRACE
            Trace(2,"\nsend broadcast to bts , case 2 ");
#endif
            PutToMacSendQueue_cpe(card_id,header->len,pkt_data);
        }

        //add to mac addr table
        prec=new mac_record_t;
        memcpy(prec->mac_addr,p+6,6);
        prec->card_id=card_id;
        g_mac_table->Insert(prec);
    }
}



void PutToMacSendQueue_bts(int card_id,int len,UINT8 *pkt_data)
{
#ifdef TEST_UTDM
	CTaskCPESM::Egress(len, pkt_data);
#else
    if (card_id != 0xFFFF)
        {
        PutToCard(card_id, len, pkt_data);
        }
#endif
    return;
}

void ProcessMacPacket_bts(pcap_pkthdr *header,u_char *pkt_data)
{
    int i,n;
    mac_record_t *prec;
    UINT8 *p;
    char str[200];

    //

    if(header->caplen!=header->len)
    {
        return;
    }

    p=pkt_data;

    {
        BinToHex((u_char*)p,20,(u_char*)str);
#ifdef USE_TRACE
        Trace(1,"\n%s %d : %s",__FILE__,__LINE__,str);
#endif
    }

    //p+6 is source mac addr
    prec=g_mac_table->Find((const unsigned char *)p+6);
    if(prec)
    {
        if(prec->card_id!=g_bts_card_id)
        {
            //this packet comes from one of cpe(#card_id), discard it
#ifdef USE_TRACE
            Trace(2,"\nLn367: bts discard a packet : %d",prec->card_id);
#endif
        }
        else
        {
            //bts side
            {
                BinToHex((u_char*)p,6,(u_char*)str);
#ifdef USE_TRACE
                Trace(2,"\nLn353: bts : dest mac addr : %s",str);
#endif
            }
            prec=g_mac_table->Find((const unsigned char *)p);//p is dest mac addr
            if(prec)
            {
                //send to cpe
                if(prec->card_id != g_bts_card_id)
                {
                    PutToMacSendQueue_bts(prec->card_id,header->len,pkt_data);
                }
            }
            else
            {
                //process broadcast packet
                if(memcmp(p,g_broadcast_mac_addr,6)==0)
                {
#ifdef USE_TRACE
                    Trace(2,"\nsend broadcast mac pac to all of cpes ...");
#endif
                    n=get_adpter_num();
                    for(i=1;i<n;i++)
                    {
                        if(i==g_bts_card_id)
                            continue;
                        PutToMacSendQueue_bts(i,header->len,pkt_data);
                    }
                }
                //do not konw how to dispatch it, discard
            }
        }
    }
    else
    {

        //add to mac addr table
        prec=new mac_record_t;
        memcpy(prec->mac_addr,p+6,6);
        prec->card_id=g_bts_card_id;
        g_mac_table->Insert(prec);

        //process broadcast packet
        if(memcmp(p,g_broadcast_mac_addr,6)==0)
        {
#ifdef USE_TRACE
            Trace(2,"\nsend broadcast mac pac to all of cpes ...");
#endif
            n=get_adpter_num();
            for(i=1;i<n;i++)
            {
                if(i==g_bts_card_id)
                    continue;
                PutToMacSendQueue_bts(i,header->len,pkt_data);
            }
        }

    }
}


void ether_pcap()
{
	int i,r;
	pcap_t *pcap;
	struct pcap_pkthdr *header;
	u_char *pkt_data; 
	//
#ifndef M_TGT_L3
    //不收BTS发送出来的包
	for(i=0; i<100; i++)
	{
		r = pcap_next_ex( get_adapter(g_bts_card_id),&header, (const u_char **)(&pkt_data) );
		if(r<=0)
			break;
		ProcessMacPacket_bts(header, pkt_data);
	}

	for(int card_id=1;card_id<get_adpter_num();card_id++)
	{
		if(card_id==g_bts_card_id)
			continue;//skip bts card id
		pcap=get_adapter(card_id);
		/* Retrieve the packets */
		for(i=0;i<10;i++)
		{
			r = pcap_next_ex( pcap,&header, (const u_char **)(&pkt_data) );
			if(r<=0)
				break;
			ProcessMacPacket_cpe(card_id,header, pkt_data);	// for test
		}
	}
#else
#ifdef TEST_UTDM
	for(i=0; i<100; i++)
	{
		r = pcap_next_ex( get_adapter(g_bts_card_id),&header, (const u_char **)(&pkt_data) );
		if(r<=0)
			break;
		ProcessMacPacket_bts(header, pkt_data);
	}
#endif
	for(int card_id=1;card_id<get_adpter_num();card_id++)
	{
		if(card_id==g_bts_card_id)
			continue;//skip bts card id
		pcap=get_adapter(card_id);
		/* Retrieve the packets */
		for(i=0;i<100;i++)
		{
			r = pcap_next_ex( pcap,&header, (const u_char **)(&pkt_data) );
			if(r<=0)
				break;
			ProcessMacPacket_cpe(card_id,header, pkt_data);	// for test
		}
#ifdef USE_TRACE
		Trace(0,"\nLn164: cpe: i = %d",i);
#endif
    }
#endif
}
