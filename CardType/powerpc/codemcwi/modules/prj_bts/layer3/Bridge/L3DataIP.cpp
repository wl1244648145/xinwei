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
#include "L3DataIPHdrCompressDef.h"

/*
 * This routine initializes the state structure for both the transmit and
 * receive halves of some serial line.  It must be called each time the
 * line is brought up.
 */
void
sl_compress_init(comp)
    struct slcompress *comp;
{
    register u_int i;
    register struct cstate *tstate = comp->tstate;

    /*
     * Clean out any junk left from the last time line was used.
     */
    bzero((char *) comp, sizeof(*comp));
    /*
     * Link the transmit states into a circular list.
     */
    for (i = MAX_STATES - 1; i > 0; --i) {
         tstate[i].cs_id = i;
         tstate[i].cs_next = &tstate[i - 1];
    }
    tstate[0].cs_next = &tstate[MAX_STATES - 1];
    tstate[0].cs_id = 0;
    comp->last_cs = &tstate[0];
    /*
     * Make sure we don't accidentally do CID compression
     * (assumes MAX_STATES < 255).
     */
    comp->last_recv = 255;
    comp->last_xmit = 255;
}


/*
 * This routine looks daunting but isn't really.  The code splits into four
 * approximately equal sized sections:  The first quarter manages a
 * circularly linked, least-recently-used list of `active' TCP
 * connections./47/  The second figures out the sequence/ack/window/urg
 * changes and builds the bulk of the compressed packet.  The third handles
 * the special-case encodings.  The last quarter does packet ID and
 * connection ID encoding and replaces the original packet header with the
 * compressed header.

 * The arguments to this routine are a pointer to a packet to be
 * compressed, a pointer to the compression state data for the serial line,
 * and a flag which enables or disables connection id (C bit) compression.

 * Compression is done `in-place' so, if a compressed packet is created,
 * both the start address and length of the incoming packet (the off and
 * len fields of m) will be updated to reflect the removal of the original
 * header and its replacement by the compressed header.  If either a
 * compressed or uncompressed packet is created, the compression state is
 * updated.  This routines returns the packet type for the transmit framer
 * (TYPE_IP, TYPE_UNCOMPRESSED_TCP or TYPE_COMPRESSED_TCP).

 * Because 16 and 32 bit arithmetic is done on various header fields, the
 * incoming IP packet must be aligned appropriately (e.g., on a SPARC, the
 * IP header is aligned on a 32-bit boundary).  Substantial changes would
 * have to be made to the code below if this were not true (and it would
 * probably be cheaper to byte copy the incoming header to somewhere
 * correctly aligned than to make those changes).

 * Note that the outgoing packet will be aligned arbitrarily (e.g., it
 * could easily start on an odd-byte boundary).
 * RETURNS:
 *          packet types.
 *          new pointer to the incoming packet
 */
u_char
sl_compress_tcp(pkt, comp, compress_cid)
    register struct ip **pkt;
    struct slcompress  *comp;
    int compress_cid;
{
    register struct cstate *cs = comp->last_cs->cs_next;
    register struct ip *ip = *pkt/*mtod(m, struct ip *)*/;
    register u_int hlen = ip->ip_hl;
    register struct tcphdr *oth;       /* last TCP header */
    register struct tcphdr *th;        /* current TCP header */
    register u_int deltaS, deltaA;     /* general purpose temporaries */
    register u_int changes = 0;        /* change mask */
    u_char new_seq[16];                /* changes from last to current */
    register u_char *cp = new_seq;

    /*
     * Bail if this is an IP fragment or if the TCP packet isn't
     * `compressible' (i.e., ACK isn't set or some other control bit is
     * set).  (We assume that the caller has already made sure the packet
     * is IP proto TCP).
     */
    if ((ip->ip_off & htons(0x3fff))/* || m->m_len < 40*/)
         return (TYPE_IP);

    th = (struct tcphdr *) & ((int *) ip)[hlen];
    if ((th->th_flags & (TH_SYN | TH_FIN | TH_RST | TH_ACK)) != TH_ACK)
         return (TYPE_IP);

    /*
     * Packet is compressible -- we're going to send either a
     * COMPRESSED_TCP or UNCOMPRESSED_TCP packet.  Either way we need to
     * locate (or create) the connection state.  Special case the most
     * recently used connection since it's most likely to be used again &
     * we don't have to do any reordering if it's used.
     */
    if (ip->ip_src.s_addr != cs->cs_ip.ip_src.s_addr ||
        ip->ip_dst.s_addr != cs->cs_ip.ip_dst.s_addr ||
        *(int *) th != ((int *) &cs->cs_ip)[cs->cs_ip.ip_hl]) {

         /*
          * Wasn't the first -- search for it.
          *
          * States are kept in a circularly linked list with last_cs
          * pointing to the end of the list.  The list is kept in lru
          * order by moving a state to the head of the list whenever
          * it is referenced.  Since the list is short and,
          * empirically, the connection we want is almost always near
          * the front, we locate states via linear search.  If we
          * don't find a state for the datagram, the oldest state is
          * (re-)used.
          */
         register struct cstate *lcs;
         register struct cstate *lastcs = comp->last_cs;

         do {
              lcs = cs;
              cs = cs->cs_next;
              if (ip->ip_src.s_addr == cs->cs_ip.ip_src.s_addr
                  && ip->ip_dst.s_addr == cs->cs_ip.ip_dst.s_addr
                  && *(int *) th == ((int *) &cs->cs_ip)[cs->cs_ip.ip_hl])
                   goto found;
         } while (cs != lastcs);

         /*
          * Didn't find it -- re-use oldest cstate.  Send an
          * uncompressed packet that tells the other side what
          * connection number we're using for this conversation. Note
          * that since the state list is circular, the oldest state
          * points to the newest and we only need to set last_cs to
          * update the lru linkage.
          */
         comp->last_cs = lcs;
         hlen += th->th_off;
         hlen <<= 2;
         goto uncompressed;

found:
         /* Found it -- move to the front on the connection list. */
         if (lastcs == cs)
              comp->last_cs = lcs;
         else {
              lcs->cs_next = cs->cs_next;
              cs->cs_next = lastcs->cs_next;
              lastcs->cs_next = cs;
         }
    }
    /*
     * Make sure that only what we expect to change changed. The first
     * line of the `if' checks the IP protocol version, header length &
     * type of service.  The 2nd line checks the "Don't fragment" bit.
     * The 3rd line checks the time-to-live and protocol (the protocol
     * check is unnecessary but costless).  The 4th line checks the TCP
     * header length.  The 5th line checks IP options, if any.  The 6th
     * line checks TCP options, if any.  If any of these things are
     * different between the previous & current datagram, we send the
     * current datagram `uncompressed'.
     */
    oth = (struct tcphdr *) & ((int *) &cs->cs_ip)[hlen];
    deltaS = hlen;
    hlen += th->th_off;
    hlen <<= 2;

    if (((u_short *) ip)[0] != ((u_short *) &cs->cs_ip)[0] ||
        ((u_short *) ip)[3] != ((u_short *) &cs->cs_ip)[3] ||
        ((u_short *) ip)[4] != ((u_short *) &cs->cs_ip)[4] ||
        th->th_off != oth->th_off ||
        (deltaS > 5 && BCMP(ip + 1, &cs->cs_ip + 1, (deltaS - 5) << 2)) ||
        (th->th_off > 5 && BCMP(th + 1, oth + 1, (th->th_off - 5) << 2)))
         goto uncompressed;

    /*
     * Figure out which of the changing fields changed.  The receiver
     * expects changes in the order: urgent, window, ack, seq.
     */
    if (th->th_flags & TH_URG) {
         deltaS = ntohs(th->th_urp);
         ENCODEZ(deltaS);
         changes |= NEW_U;
    } else if (th->th_urp != oth->th_urp)
         /*
          * argh! URG not set but urp changed -- a sensible
          * implementation should never do this but RFC793 doesn't
          * prohibit the change so we have to deal with it.
          */
         goto uncompressed;

    if (deltaS = (u_short) (ntohs(th->th_win) - ntohs(oth->th_win))) {
         ENCODE(deltaS);
         changes |= NEW_W;
    }
    if (deltaA = ntohl(th->th_ack) - ntohl(oth->th_ack)) {
         if (deltaA > 0xffff)
              goto uncompressed;
         ENCODE(deltaA);
         changes |= NEW_A;
    }
    if (deltaS = ntohl(th->th_seq) - ntohl(oth->th_seq)) {
         if (deltaS > 0xffff)
              goto uncompressed;
         ENCODE(deltaS);
         changes |= NEW_S;
    }
    /*
     * Look for the special-case encodings.
     */
    switch (changes) {

    case 0:
         /*
          * Nothing changed. If this packet contains data and the last
          * one didn't, this is probably a data packet following an
          * ack (normal on an interactive connection) and we send it
          * compressed.  Otherwise it's probably a retransmit,
          * retransmitted ack or window probe.  Send it uncompressed
          * in case the other side missed the compressed version.
          */
         if (ip->ip_len != cs->cs_ip.ip_len &&
             ntohs(cs->cs_ip.ip_len) == hlen)
              break;

         /* (fall through) */

    case SPECIAL_I:
    case SPECIAL_D:
         /*
          * Actual changes match one of our special case encodings --
          * send packet uncompressed.
          */
         goto uncompressed;

    case NEW_S | NEW_A:
         if (deltaS == deltaA &&
             deltaS == ntohs(cs->cs_ip.ip_len) - hlen) {
              /* special case for echoed terminal traffic */
              changes = SPECIAL_I;
              cp = new_seq;
         }
         break;

    case NEW_S:
         if (deltaS == ntohs(cs->cs_ip.ip_len) - hlen) {
              /* special case for data xfer */
              changes = SPECIAL_D;
              cp = new_seq;
         }
         break;
    }
    deltaS = ntohs(ip->ip_id) - ntohs(cs->cs_ip.ip_id);
    if (deltaS != 1) {
         ENCODEZ(deltaS);
         changes |= NEW_I;
    }
    if (th->th_flags & TH_PUSH)
         changes |= TCP_PUSH_BIT;
    /*
     * Grab the cksum before we overwrite it below.  Then update our
     * state with this packet's header.
     */
    deltaA = ntohs(th->th_sum);
    BCOPY(ip, &cs->cs_ip, hlen);

    /*
     * We want to use the original packet as our compressed packet. (cp -
     * new_seq) is the number of bytes we need for compressed sequence
     * numbers.  In addition we need one byte for the change mask, one
     * for the connection id and two for the tcp checksum. So, (cp -
     * new_seq) + 4 bytes of header are needed.  hlen is how many bytes
     * of the original packet to toss so subtract the two to get the new
     * packet size.
     */
    deltaS = cp - new_seq;
    cp = (u_char *) ip;
    if (compress_cid == 0 || comp->last_xmit != cs->cs_id) {
         comp->last_xmit = cs->cs_id;
         hlen -= deltaS + 4;
         cp += hlen;
         *pkt = cp;
         *cp++ = changes | NEW_C;
         *cp++ = cs->cs_id;
    } else {
         hlen -= deltaS + 3;
         cp += hlen;
         *pkt = cp;
         *cp++ = changes;
    }
    //m->m_len -= hlen;
    //m->m_off += hlen;
    *cp++ = deltaA >> 8;
    *cp++ = deltaA;
    BCOPY(new_seq, cp, deltaS);
    return (TYPE_COMPRESSED_TCP);

uncompressed:
    /*
     * Update connection state cs & send uncompressed packet
     * ('uncompressed' means a regular ip/tcp packet but with the
     * 'conversation id' we hope to use on future compressed packets in
     * the protocol field).
     */
    BCOPY(ip, &cs->cs_ip, hlen);
    ip->ip_p = cs->cs_id;
    comp->last_xmit = cs->cs_id;
    return (TYPE_UNCOMPRESSED_TCP);
}



/*
 *  This routine decompresses a received packet.  It is called with a
 *  pointer to the packet, the packet length and type, and a pointer to the
 *  compression state structure for the incoming serial line.  It returns a
 *  pointer to the resulting packet or zero if there were errors in the
 *  incoming packet.  If the packet is COMPRESSED_TCP or UNCOMPRESSED_TCP,
 *  the compression state will be updated.
 *  there must be 128 bytes of free space in front of bufp to allow room
 *  for the reconstructed IP and TCP headers.  The reconstructed packet 
 *  will be aligned on a 32-bit boundary.
 *
 *  RETURNS:
 *        0 or pointer to the resulting packet
 */
u_char *
sl_uncompress_tcp(bufp, len, type, comp)
    u_char *bufp;
    int len;
    u_int type;
    struct slcompress *comp;
{
    register u_char *cp;
    register u_int hlen, changes;
    register tcphdr *th;
    register struct cstate *cs;
    register struct ip *ip;

    switch (type) {

    case TYPE_IP:
         return (bufp);

    case TYPE_UNCOMPRESSED_TCP:
         /*
          * Locate the saved state for this connection.  If the state
          * index is legal, clear the 'discard' flag.
          */
         ip = (struct ip *) bufp;
         if (ip->ip_p >= MAX_STATES)
              goto bad;

         cs = &comp->rstate[comp->last_recv = ip->ip_p];
         comp->flags &= ~SLF_TOSS;
         /*
          * Restore the IP protocol field then save a copy of this
          * packet header.  (The checksum is zeroed in the copy so we
          * don't have to zero it each time we process a compressed
          * packet.
          */
         ip->ip_p = IPPROTO_TCP;
         hlen = ip->ip_hl;
         hlen += ((struct tcphdr *) & ((int *) ip)[hlen])->th_off;
         hlen <<= 2;
         BCOPY(ip, &cs->cs_ip, hlen);
         cs->cs_ip.ip_sum = 0;
         cs->cs_hlen = hlen;
         return (bufp);

    case TYPE_COMPRESSED_TCP:
         break;

    case TYPE_ERROR:
    default:
         goto bad;
    }
    /* We've got a compressed packet. */
    cp = bufp;
    changes = *cp++;
    if (changes & NEW_C) {
         /*
          * Make sure the state index is in range, then grab the
          * state. If we have a good state index, clear the 'discard'
          * flag.
          */
         if (*cp >= MAX_STATES)
              goto bad;

         comp->flags &= ~SLF_TOSS;
         comp->last_recv = *cp++;
    } else {
         /*
          * This packet has an implicit state index.  If we've had a
          * line error since the last time we got an explicit state
          * index, we have to toss the packet.
          */
         if (comp->flags & SLF_TOSS)
              return ((u_char *) 0);
    }
    /*
     * Find the state then fill in the TCP checksum and PUSH bit.
     */
    cs = &comp->rstate[comp->last_recv];
    hlen = cs->cs_ip.ip_hl << 2;
    th = (struct tcphdr *) & ((u_char *) &cs->cs_ip)[hlen];
    th->th_sum = htons((*cp << 8) | cp[1]);
    cp += 2;
    if (changes & TCP_PUSH_BIT)
         th->th_flags |= TH_PUSH;
    else
         th->th_flags &= ~TH_PUSH;

    /*
     * Fix up the state's ack, seq, urg and win fields based on the
     * changemask.
     */
    switch (changes & SPECIALS_MASK) {
    case SPECIAL_I:
         {
         register u_int i = ntohs(cs->cs_ip.ip_len) - cs->cs_hlen;
         th->th_ack = htonl(ntohl(th->th_ack) + i);
         th->th_seq = htonl(ntohl(th->th_seq) + i);
         }
         break;

    case SPECIAL_D:
         th->th_seq = htonl(ntohl(th->th_seq) + ntohs(cs->cs_ip.ip_len)
                      - cs->cs_hlen);
         break;

    default:
         if (changes & NEW_U) {
              th->th_flags |= TH_URG;
              DECODEU(th->th_urp)
         } else
              th->th_flags &= ~TH_URG;
         if (changes & NEW_W)
              DECODES(th->th_win)
         if (changes & NEW_A)
              DECODEL(th->th_ack)
         if (changes & NEW_S)
              DECODEL(th->th_seq)
         break;
    }
    /* Update the IP ID */
    if (changes & NEW_I)
         DECODES(cs->cs_ip.ip_id)
    else
         cs->cs_ip.ip_id = htons(ntohs(cs->cs_ip.ip_id) + 1);

    /*
     * At this point, cp points to the first byte of data in the packet.
     * If we're not aligned on a 4-byte boundary, copy the data down so
     * the IP & TCP headers will be aligned.  Then back up cp by the
     * TCP/IP header length to make room for the reconstructed header (we
     * assume the packet we were handed has enough space to prepend 128
     * bytes of header).  Adjust the lenth to account for the new header
     * & fill in the IP total length.
     */
    len -= (cp - bufp);
    if (len < 0)
         /*
          * we must have dropped some characters (crc should detect
          * this but the old slip framing won't)
          */
         goto bad;

    if ((int) cp & 3) {
         if (len > 0)
              OVBCOPY(cp, (int) cp & ~3, len);
         cp = (u_char *) ((int) cp & ~3);
    }
    cp -= cs->cs_hlen;
    len += cs->cs_hlen;
    cs->cs_ip.ip_len = htons(len);
    BCOPY(&cs->cs_ip, cp, cs->cs_hlen);

    /* recompute the ip header checksum */
    {
         register u_short *bp = (u_short *) cp;
         for (changes = 0; hlen > 0; hlen -= 2)
              changes += *bp++;
         changes = (changes & 0xffff) + (changes >> 16);
         changes = (changes & 0xffff) + (changes >> 16);
         ((struct ip *) cp)->ip_sum = ~changes;
    }
    return (cp);

bad:
    comp->flags |= SLF_TOSS;
    return ((u_char *) 0);
}


void
compress(CComMessage *pComMsg, slcompress *comp)
{
    if ((NULL == pComMsg)||(NULL == comp))
        {
        return;
        }
    EtherHdr *etherpkt = pComMsg->GetDataPtr();
    int pktlen = pComMsg->GetDataLength();
    ip  *ippkt = (ip*)((char*)etherpkt + sizeof(EtherHdr));
    if ((ippkt->ip_p == M_PROTOCOL_TYPE_TCP) && (ippkt->ip_len >= 40))
        {
        ip *oldpkt = ippkt;
        char type = sl_compress_tcp(&ippkt, comp, 1);
        *((char*)ippkt) |= type;
        switch (type) 
            {
            case TYPE_COMPRESSED_TCP:
#if 1
                //同一个TCP连接,ether header也可以不传输
                int deltaLen = (int)ippkt - (int)etherpkt;
#else
                int deltaLen = (int)ippkt - (int)oldpkt;
                //copy ether header backward deltaLen Bytes.
                char *p = (char*)etherpkt + sizeof(EtherHdr) - 1;
                for ( UINT8 idx = sizeof(EtherHdr); idx > 0; --idx, --p )
                    {
                    *(p + deltaLen) = *p;
                    }
#endif
                pComMsg->SetDataPtr(ippkt);
                pComMsg->SetDataLength(pktlen - deltaLen);
            }
        }
}

void
uncompress(CComMessage *pComMsg, slcompress *comp)
{
    if ((NULL == pComMsg)||(NULL == comp))
        {
        return;
        }
    char *pkt   = pComMsg->GetDataPtr();
    int  pktlen = pComMsg->GetDataLength();

    register char type = *pkt & 0xf0;
    if (type != (IPVERSION << 4))
        {
        if (type & 0x80)
            type = TYPE_COMPRESSED_TCP;
        else if (type == TYPE_UNCOMPRESSED_TCP)
            *pkt &= 0x4f;

        /*
        * we've got something that's not an IP packet.
        * If compression is enabled, try to uncompress
        * it.  Otherwise, if `auto-enable' compression
        * is on and it's a reasonable packet,
        * uncompress it then enable compression.
        * Otherwise, drop it.
        */

        /*
         * If we've missed a packet, we must toss subsequent compressed
         * packets which don't have an explicit connection ID.
         */
#if 0
        if ((sc->sc_flags & SL_COMPRESS) || ((sc->sc_flags & SL_COMPRESS_RX) &&
            (type == TYPE_UNCOMPRESSED_TCP) && (sc->sc_ilen >= 40)))
#else
        if ((type == TYPE_COMPRESSED_TCP) || (type == TYPE_UNCOMPRESSED_TCP))
#endif
            {
            if (sl_uncompress_tcp ((u_char *)pkt, pktlen, (u_int) type, comp) <= 0)
                {
                return (NULL);
                }
            }
        else
            {
            return (NULL);
            }
        }

#if 0
    ip  *ippkt = (ip*)((char*)etherpkt + sizeof(EtherHdr));
    if ((ippkt->ip_p == M_PROTOCOL_TYPE_TCP) && (ippkt->ip_len >= 40))
        {
        ip *oldpkt = ippkt;
        char type = sl_compress_tcp(&ippkt, comp, 1);
        switch (type) 
            {
            case TYPE_COMPRESSED_TCP:
#if 1
                //同一个TCP连接,ether header也可以不传输
                int deltaLen = (int)ippkt - (int)etherpkt;
#else
                int deltaLen = (int)ippkt - (int)oldpkt;
                //copy ether header backward deltaLen Bytes.
                char *p = (char*)etherpkt + sizeof(EtherHdr) - 1;
                for ( UINT8 idx = sizeof(EtherHdr); idx > 0; --idx, --p )
                    {
                    *(p + deltaLen) = *p;
                    }
#endif
                pComMsg->SetDataPtr(ippkt);
                pComMsg->SetDataLength(pktlen - deltaLen);
            }
        }
#endif
}

