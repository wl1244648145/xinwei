#ifndef USDPAA_SDK1_PARSE_RESULT_H
#define USDPAA_SDK1_PARSE_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif


#define FMAN_PARSE_RESULT_OFFSET	16
#define ETH_HDR_LEN			14
#define IP_HEADER_LENGTH_NO_OPTIONS	20

struct output_parse_result_t {
        uint32_t pc:2;          /* 0x00 */
        uint32_t port_id:6;
        uint8_t shimr;          /* 0x01 */
        uint16_t l2r;           /* 0x02 */
        uint16_t l3r;           /* 0x04 */
        uint8_t l4r;            /* 0x06 */
        uint8_t classification_planid;  /* 0x07 */
        uint16_t nxt_hdr_type;  /* 0x08 */
        uint16_t checksum;      /* 0x0A */
        uint32_t lineup_confirmation_vector;    /* 0x0c */
        uint8_t shim1O;         /* 0x10 */
        uint8_t shim2O;         /* 0x11 */
        uint8_t shim3O;         /* 0x12 */
        uint8_t ethO;           /* 0x13 */
        uint8_t llc_snapO;      /* 0x14 */
        uint8_t vlan1O;         /* 0x15 */
        uint8_t vlan2O;         /* 0x16 */
        uint8_t lastEtype0;     /* 0x17 */
        uint8_t pppoeO;         /* 0x18 */
        uint8_t mpls1O;         /* 0x19 */
        uint8_t mpls2O;         /* 0x1a */
        uint8_t ipO;            /* 0x1B */
        uint8_t ipO_or_minencapO;       /* 0x1C */
        uint8_t gre;            /* 0x1D */
        uint8_t l4O;            /* 0x1E */
        uint8_t nxtHdr0;        /* 0x1F */
} __attribute__ ((__packed__));


typedef struct fd_buffer_head_s {
	uint8_t reserved[FMAN_PARSE_RESULT_OFFSET];
	struct output_parse_result_t parse_result;
	//uint8_t reserved1[FMAN_PARSE_RESULT_OFFSET];
} __packed fd_buffer_head_t;


static inline void fman_chksm_enable(void *addr)
{
	fd_buffer_head_t *buf = addr;

	//assert(NULL != buf);

	/* Pre-initialize the annotations section */
        /* Set the offset to the ETH header from the start of the frame. */
        buf->parse_result.ethO = 0;
        /* Set the offset to the VLAN header from the start of the frame. Here set to 0 for non-VLAN case. */
        buf->parse_result.vlan1O = 0;
        /* Set the offset to the IP header from the start of the frame. */
        buf->parse_result.ipO = ETH_HDR_LEN;
        /* Set the offset to the UDP header from the start of the frame. */
        buf->parse_result.l4O = ETH_HDR_LEN + IP_HEADER_LENGTH_NO_OPTIONS;
        /* Set L2 Type: set first bit meaning "Ethernet present" and "VLAN present".
         See $24.21.4.4.6 from P4080RM_RevG.pdf */
        buf->parse_result.l2r = 0x8000;
        /* Set L3 Type: set first bit meaning "First IP Present IPv4".
           See $24.21.4.4.13 from P4080RM_RevG.pdf  */
        buf->parse_result.l3r = 0x8000;
        /* Set L4Type: set value 010 in first 3 bytes meaning "UDP present".
           See $24.21.4.4.20 from P4080RM_RevG.pdf  */
        buf->parse_result.l4r = 0x40;

}


#ifdef __cplusplus
}
#endif

#endif	//USDPAA_SDK1_PARSE_RESULT_H

