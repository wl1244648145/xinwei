/*
 *  sha1.h
 *
 *  Description:
 *      This is the header file for code which implements the Secure
 *      Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 *      April 17, 1995.
 *
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 *
 */

#ifndef _SHA1_H_
#define _SHA1_H_

/*
 * 
 *  name                    meaning
 *  SHA_uint32_t            unsigned 32 bit integer
 *  SHA_uint8_t              unsigned 8 bit integer (i.e., unsigned char)
 *  SHA_int_least16_t       integer of >= 16 bits
 *
 */

typedef unsigned int SHA_uint32_t;
typedef unsigned char SHA_uint8_t;
typedef int SHA_int_least16_t;

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};

#endif
#define SHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    SHA_uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */
    SHA_uint32_t Length_Low;            /* Message length in bits      */
    SHA_uint32_t Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    SHA_int_least16_t Message_Block_Index;
    SHA_uint8_t Message_Block[64];      /* 512-bit message blocks      */
    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;


#ifdef __cplusplus
extern "C" 
{
#endif

/*
 *  Function Prototypes
 */

int SHA1Reset(  SHA1Context *);
int SHA1Input(  SHA1Context *,
                const SHA_uint8_t *,
                unsigned int);
int SHA1Result( SHA1Context *,
                SHA_uint8_t Message_Digest[SHA1HashSize]);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif

