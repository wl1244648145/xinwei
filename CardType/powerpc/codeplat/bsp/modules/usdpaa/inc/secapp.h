/******************************************************************************


 **************************************************************************/
#ifndef __SECAPP_H
#define __SECAPP_H

#include "fqid.h"


#define FQID_TO_SEC_CORE0EN_CORE0DE		(FQID_SEC_BASE + 0)
#define FQID_FROM_SEC_CORE0EN_CORE0DE	    (FQID_SEC_BASE + 1)
#define FQID_TO_SEC_CORE0EN_CORE1DE		(FQID_SEC_BASE + 2)
#define FQID_FROM_SEC_CORE1EN_CORE0DE	    (FQID_SEC_BASE + 3)

#define FQID_TO_SEC_CORE1EN_CORE1DE		(FQID_SEC_BASE + 4)
#define FQID_FROM_SEC_CORE1EN_CORE1DE	    (FQID_SEC_BASE + 5)
#define FQID_TO_SEC_CORE1EN_CORE0DE		(FQID_SEC_BASE + 6)
#define FQID_FROM_SEC_CORE0EN_CORE1DE	    (FQID_SEC_BASE + 7)

#define FQID_TO_SEC_F8_CORE0		(FQID_SEC_BASE + 8)
#define FQID_FROM_SEC_F8_CORE0	    (FQID_SEC_BASE + 9)
#define FQID_TO_SEC_F9_CORE0		(FQID_SEC_BASE + 10)
#define FQID_FROM_SEC_F9_CORE0	    (FQID_SEC_BASE + 11)
#define FQID_TO_SEC_F9F8_CORE0		(FQID_SEC_BASE + 12)
#define FQID_FROM_SEC_F9F8_CORE0	    (FQID_SEC_BASE + 13)




struct preheader_s {
	union {
		uint32_t word;
		struct {
			uint16_t rsvd63_48;
			unsigned int rsvd47_39:9;
			unsigned int idlen:7;
		} field;
	}  hi;

	union {
		uint32_t word;
		struct {
			unsigned int rsvd31_30:2;
			unsigned int fsgt:1;
			unsigned int lng:1;
			unsigned int offset:2;
			unsigned int abs:1;
			unsigned int add_buf:1;
			uint8_t pool_id;
			uint16_t pool_buffer_size;
		} field;
	}  lo;
};


/**
\brief		Scatter Gather Entry

\details 	Speicifies the the Scatter Gather Format related information 
*/
struct sg_entry_t {
	uint16_t reserved_zero;
	uint16_t addr_hi;	/**< Memory Address of the start of the buffer - hi*/
	uint32_t addr_lo;	/**< Memory Address - lo*/
	unsigned int extension:1;
	unsigned int final:1;
	unsigned int length:30;	/**< Length of the data in the frame */
	uint8_t reserved_zero2;
	uint8_t bpid;		/**< Buffer Pool Id */
	unsigned int reserved_offset:3;
	unsigned int offset:13;
};


#endif /* __SECAPP_H */
