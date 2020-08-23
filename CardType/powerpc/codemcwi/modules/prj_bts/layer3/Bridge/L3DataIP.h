/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are
 * permitted provided that the above copyright notice and this
 * paragraph are duplicated in all such forms and that any
 * documentation, advertising materials, and other materials
 * related to such distribution and use acknowledge that the
 * software was developed by the University of California,
 * Berkeley.  The name of the University may not be used to
 * endorse or promote products derived from this software
 * without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#ifndef __INCDataIpHdrComprh
#define __INCDataIpHdrComprh

#pragma pack (1)

#define MAX_STATES 16   /* must be >2 and <255 */
#define MAX_HDR 128     /* max TCP+IP hdr length (by protocol def) */

/* packet types */
#define TYPE_IP 0x40
#define TYPE_UNCOMPRESSED_TCP 0x70
#define TYPE_COMPRESSED_TCP 0x80
#define TYPE_ERROR 0x00 /* this is not a type that ever appears on
                        * the wire.  The receive framer uses it to
                        * tell the decompressor there was a packet
                        * transmission error. */
/*
* Bits in first octet of compressed packet
*/

/* flag bits for what changed in a packet */

#define NEW_C  0x40
#define NEW_I  0x20
#define TCP_PUSH_BIT 0x10

#define NEW_S  0x08
#define NEW_A  0x04
#define NEW_W  0x02
#define NEW_U  0x01

/* reserved, special-case values of above */
#define SPECIAL_I (NEW_S|NEW_W|NEW_U)        /* echoed interactive traffic */
#define SPECIAL_D (NEW_S|NEW_A|NEW_W|NEW_U)  /* unidirectional data */
#define SPECIALS_MASK (NEW_S|NEW_A|NEW_W|NEW_U)

struct ip {
#if 0
    //С�ֽ���
    u_int    ip_hl:4,         /* header length */
              ip_v:4,         /* version */
#else
    //���ֽ���
     u_int    ip_v:4,         /* version */
             ip_hl:4,         /* header length */
#endif
            ip_tos:8,       /* type of service */
            ip_len:16;      /* total length */
    u_short ip_id;          /* identification */
    short   ip_off;         /* fragment offset field */
#define IP_DF 0x4000            /* dont fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
    u_char  ip_ttl;         /* time to live */
    u_char  ip_p;           /* protocol */
    u_short ip_sum;         /* checksum */
    struct  in_addr ip_src,ip_dst;  /* source and dest address */
};

struct tcphdr {
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
    UINT16 th_sport;       /* source port */
    UINT16 th_dport;       /* destination port */
    UINT32 th_seq;         /* sequence number */
    UINT32 th_ack;         /* acknowledgement number */
#if 1
    //���ֽ���
    UINT32 th_off:4,       /* data offset */
            th_x2:4,       /* (unused) */
#else
    //С�ֽ���
    u_int   th_x2:4,        /* (unused) */
           th_off:4,       /* data offset */
#endif
         th_flags:8,
           th_win:16;      /* window */
    UINT16 th_sum;         /* checksum */
    UINT16 th_urp;         /* urgent pointer */
};

/*
* "state" data for each active tcp conversation on the wire.  This is
* basically a copy of the entire IP/TCP header from the last packet together
* with a small identifier the transmit & receive ends of the line use to
* locate saved header.
*/
struct cstate 
{
    struct cstate *cs_next;  /* next most recently used cstate (xmit only) */
    u_short cs_hlen;         /* size of hdr (receive only) */
    u_char  cs_id;            /* connection # associated with this state */
    u_char  cs_filler;
    union 
    {
         char hdr[MAX_HDR];
         struct ip csu_ip;   /* ip/tcp hdr from most recent packet */
    } slcs_u;
};
#define cs_ip  slcs_u.csu_ip

#define cs_hdr slcs_u.csu_hdr

/*
* all the state data for one serial line (we need one of these per line).
*/
struct slcompress {
    struct cstate *last_cs;            /* most recently used tstate */
    u_char last_recv;                  /* last rcvd conn. id */
    u_char last_xmit;                  /* last sent conn. id */
    u_short flags;
    struct cstate tstate[MAX_STATES];  /* xmit connection states */
    struct cstate rstate[MAX_STATES];  /* receive connection states */
};

/* flag values */
#define SLF_TOSS 1       /* tossing rcvd frames because of input err */

/*
* The following macros are used to encode and decode numbers.  They all
* assume that `cp' points to a buffer where the next byte encoded (decoded)
* is to be stored (retrieved).  Since the decode routines do arithmetic,
* they have to convert from and to network byte order.
*/

/*
* ENCODE encodes a number that is known to be non-zero.  ENCODEZ checks for
* zero (zero has to be encoded in the long, 3 byte form).
*/
#define ENCODE(n) { \
    if ((u_short)(n) >= 256) { \
         *cp++ = 0; \
         cp[1] = (n); \
         cp[0] = (n) >> 8; \
         cp += 2; \
    } else { \
         *cp++ = (n); \
    } \
}
#define ENCODEZ(n) { \
    if ((u_short)(n) >= 256 || (u_short)(n) == 0) { \
         *cp++ = 0; \
         cp[1] = (n); \
         cp[0] = (n) >> 8; \
         cp += 2; \
    } else { \
         *cp++ = (n); \
    } \
}

/*
* DECODEL takes the (compressed) change at byte cp and adds it to the
* current value of packet field 'f' (which must be a 4-byte (long) integer
* in network byte order).  DECODES does the same for a 2-byte (short) field.
* DECODEU takes the change at cp and stuffs it into the (short) field f.
* 'cp' is updated to point to the next field in the compressed header.
*/
#define DECODEL(f) { \
    if (*cp == 0) {\
         (f) = htonl(ntohl(f) + ((cp[1] << 8) | cp[2])); \
         cp += 3; \
    } else { \
         (f) = htonl(ntohl(f) + (u_long)*cp++); \
    } \
}
#define DECODES(f) { \
    if (*cp == 0) {\
         (f) = htons(ntohs(f) + ((cp[1] << 8) | cp[2])); \
         cp += 3; \
    } else { \
         (f) = htons(ntohs(f) + (u_long)*cp++); \
    } \
}
#define DECODEU(f) { \
    if (*cp == 0) {\
         (f) = htons((cp[1] << 8) | cp[2]); \
         cp += 3; \
    } else { \
         (f) = htons((u_long)*cp++); \
    } \
}

#pragma pack ()
#endif  /*__INCDataIpHdrComprh*/