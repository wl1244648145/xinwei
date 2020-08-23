#ifndef L3_DATACAP_H
#define L3_DATACAP_H

#ifndef lib_pcap_h
struct pcap_pkthdr {
    int caplen; /* length of portion present */
    int len;    /* length this packet (off wire) */
};
#endif

pcap_t* get_adapter(int card_no);

#define Max_Num_Adapter 10

/* hash table */
typedef struct hash_node_t {
    struct hash_node_t *link;
}hash_node_t;

typedef struct  {
    unsigned long
        (*hash)(const void *);	
    int (*compare)(const void *, const void *);	
    const void  *(*keyof)(const hash_node_t *);
    unsigned int	
        nbuckets;
    unsigned int	
        count;
    hash_node_t **table;
} hashtab_t;

typedef hashtab_t hash_table_t;

/***************** mac record and table class ****************/

//record a src mac addr comes from which net card(if)
typedef struct {
    hash_node_t node;
    UINT8 mac_addr[6];
    int card_id;
} mac_record_t;

#define T_MacRecordTable_HEntryNum 100
class T_MacRecordTable {
    void *h_lock;
    hash_table_t htab; 
    hash_node_t *hentry[T_MacRecordTable_HEntryNum];
public:
    T_MacRecordTable();
    ~T_MacRecordTable(){};
    bool Insert(mac_record_t *prec);
    void Delete(mac_record_t *prec);
    mac_record_t *Find(const UINT8 *addr);
};

extern T_MacRecordTable *g_mac_table;
extern u_char g_broadcast_mac_addr[6];

extern "C" int get_adpter_num();
u_char *get_net_card_mac_addr(int card_no);

extern "C" {
int init_pcap_vars();
void ether_pcap();
void PutToCard(int card_id, int len, UINT8 *pkt_data);
int init_pcap_vars();
int get_mac_addr(char *name,char *outMacAdd);
}
#endif //L3_DATACAP_H
