#include "L3DataLib.h"

int BinToHex(byte *bin, int l, byte *hex )
{
    int i;
    char c1, c2;

    for( i = 0; i < l; i ++ )
    {
        c1 = bin[ i ] & 0xf0;
        c1 = c1 >> 4;
        c2 = bin[ i ] & 0x0f;
        if( ((int)c1 >= 0x00) && ((int)c1 <= 0x09) )
            c1 = c1 + 0x30;
        else if( ((int)c1 >= 0x0A) && ((int)c1 <= 0x0F) )
            c1 = c1 + 0x37;
        if( ((int)c2 >= 0x00) && ((int)c2 <= 0x09) )
            c2 = c2 + 0x30;
        else if( ((int)c2 >= 0x0A) && ((int)c2 <= 0x0F) )
            c2 = c2 + 0x37;

        hex[ i*2 ] = c1;
        hex[ i*2+1 ] = c2;
    }
    hex[ 2*l ] =0;
    return 2*l;
}


/** Hash Tables *************************************************************/
void    HashInit(
    hashtab_t *H,
    unsigned int nbuckets,
    unsigned long (*hash)(const void *),
    int (*compare)(const void *, const void *),
    const void *(*keyof)(const hash_node_t *),
    hash_node_t **table
)
{
    unsigned int    i;

    H->count = 0;
    H->nbuckets = nbuckets;
    H->hash = hash;
    H->keyof = keyof;
    H->compare = compare;
    H->table = table;

    for( i=0; i<nbuckets; i++ )
    {
        H->table[i] = 0;
    }
}
void    HashInsert(hashtab_t *H, hash_node_t *N)
{
    unsigned int    index;

    index = (H->hash(H->keyof(N))) % (H->nbuckets);
    N->link = H->table[index];
    H->table[index] = N;
    H->count++;
}

void    HashDelete(hashtab_t *H, hash_node_t *N)
{
    unsigned int    index;
    hash_node_t *p;

    index = (H->hash(H->keyof(N))) % (H->nbuckets);
    p = H->table[index];

    if( p == N )
    {
        H->table[index] = N->link;
        N->link = 0;
        H->count--;
    }
    else
    {
        while( p && (p->link != N) )
        {
            p = p->link;
        }
        if( !p )
        {
            return;
        }
        p->link = N->link;
        N->link = 0;
        H->count--;
    }
}

unsigned int    HashCount(const hashtab_t *H)
{
    return H->count;
}

hash_node_t *HashFind(const hashtab_t *H, const void *item)
{
    unsigned int    index;
    hash_node_t *p;

    index = (H->hash(item)) % (H->nbuckets);
    p = H->table[index];

    while( p && H->compare( item, H->keyof(p) ) )
    {
        p = p->link;
    }

    return p;
}

hash_node_t *HashFirst( const hashtab_t *H )
{
    unsigned int    index;

    for( index=0; index<H->nbuckets; index++ )
    {
        if( H->table[index] )
        {
            return H->table[index];
        }
    }
    return 0;
}

hash_node_t *HashNext( const hashtab_t *H, const hash_node_t *N )
{
    unsigned int    index;

    if( N->link )
    {
        return N->link;
    }

    index = (H->hash(H->keyof(N))) % (H->nbuckets);
    index++;
    for( ; index<H->nbuckets; index++ )
    {
        if( H->table[index] )
        {
            return H->table[index];
        }
    }
    return 0;
}



