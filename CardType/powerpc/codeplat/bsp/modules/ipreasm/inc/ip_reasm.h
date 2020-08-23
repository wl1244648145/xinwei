/* ip_reasm.h */

#ifndef _IP_REASM_H_
#define _IP_REASM_H_

#define IP_CE             0x8000   /* Flag: "Congestion" */
#define IP_DF             0x4000   /* Flag: "Don't Fragment" */
#define IP_MF             0x2000   /* Flag: "More Fragments" */
#define IP_OFFSET         0x1FFF  /* "Fragment Offset" part */
#define IP_FRAG_TIME      (30 * 1000)   /* fragment lifetime */
#define UNUSED            314159
#define FREE_READ         UNUSED
#define FREE_WRITE        UNUSED
#define GFP_ATOMIC        UNUSED
#define NETDEBUG(x)
#define ETHER_ADDR_LEN    6
#define IPF_NOTF          0
#define IPF_NEW           1
#define IPF_ISF           2
/*

  Fragment cache limits. We will commit 256K at one time. Should we

  cross that limit we will prune down to 192K. This should cope with

  even the most extreme cases without allowing an attacker to

  measurably harm machine performance.

*/

#define IPFRAG_HIGH_THRESH            (256*1024)

#define IPFRAG_LOW_THRESH            (192*1024)

#define BIG_ENDIAN_BITFIELD

/**********************************/
struct   ip     
{
#if defined (LITTLE_ENDIAN_BITFIELD)    
    unsigned   char    ip_hl:4;
    unsigned   char    version:4;
#elif defined (BIG_ENDIAN_BITFIELD)    
    unsigned   char    version:4;
    unsigned   char    ip_hl:4;
#else
#error     "Unkown Endian"
#endif
    unsigned   char		tos;                    //   Type   of   service   
    unsigned   short	ip_len;				//   Total   length   of   the   packet   
    unsigned   short	ip_id;					//   Unique   identifier   
    unsigned   short	ip_off;					//   3位标志位+13报片偏移   
    unsigned   char		ttl;                    //   Time   to   live   
    unsigned   char		ip_p;                  //   Protocol   (TCP,   UDP   etc)   
    unsigned   short		checksum;               //   IP   checksum   
    unsigned int ip_src;
    unsigned int ip_dst;
};

struct	ether_header 
{
	unsigned char  ether_dhost[ETHER_ADDR_LEN];
	unsigned char  ether_shost[ETHER_ADDR_LEN];
	unsigned short ether_type;
};

/**********************************/

struct sk_buff 
{
    char *data;
    int truesize;
};


struct timer_list 
{
    struct timer_list *prev;
    struct timer_list *next;
    int expires;
    void (*function)(unsigned long arg);
    unsigned long data;
  // struct ipq *frags;
};


struct hostfrags 
{
    struct ipq *ipqueue;
    int ip_frag_mem;
    unsigned int ip;
    int hash_index;
    struct hostfrags *prev;
    struct hostfrags *next;
}g_hostfrags;

/* Describe an IP fragment. */

struct ipfrag 
{
  int offset;                 /* offset of fragment in IP datagram    */
  int end;                    /* last byte of data in datagram        */
  int len;                     /* length of this fragment              */
  struct sk_buff *skb;         /* complete received fragment           */
  unsigned char *ptr;         /* pointer into real fragment data      */
  struct ipfrag *next;          /* linked list pointers                 */
  struct ipfrag *prev;
};


/* Describe an entry in the "incomplete datagrams" queue. */

struct ipq 
{
  unsigned char *mac;             /* pointer to MAC header                */
  struct ip *iph;           /* pointer to IP header                 */
  int len;                     /* total length of original datagram    */
  short ihlen;                     /* length of the IP header              */
  short maclen;                 /* length of the MAC header             */
  struct timer_list timer;     /* when will this queue expire?         */
  struct ipfrag *fragments; /* linked list of received fragments    */
  struct hostfrags * hf;
  struct ipq *next;              /* linked list pointers                 */
  struct ipq *prev;
  // struct device *dev;       /* Device - for icmp replies */
};



#endif                                  /* _IP_REASM_H_ */

