#ifndef __L3DATA_LIB__
#define __L3DATA_LIB__
#include "pcap.h"
extern "C"
{
#include "remote-ext.h"
}
#include "pcap-bpf.h"
//#include "sockstorage.h"
#include "packet32.h"
#include "ntddndis.h"

#include "WinPCAP.h"

int BinToHex(byte *bin, int l, byte *hex );
void    HashInit(
    hashtab_t *H,
    unsigned int nbuckets,
    unsigned long (*hash)(const void *),
    int (*compare)(const void *, const void *),
    const void *(*keyof)(const hash_node_t *),
    hash_node_t **table
);
void    HashInsert(hashtab_t *H, hash_node_t *N);
void    HashDelete(hashtab_t *H, hash_node_t *N);
void    HashDelete(hashtab_t *H, hash_node_t *N);
hash_node_t *HashFind(const hashtab_t *H, const void *item);
hash_node_t *HashFirst( const hashtab_t *H );
hash_node_t *HashNext( const hashtab_t *H, const hash_node_t *N );
#endif