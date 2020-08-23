/*
 * SEC Descriptor Construction Library
 * Basic job descriptor construction
 *
 * Copyright (c) 2009, Freescale Semiconductor, Inc.
 * All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../inc/compat.h"
#include "../inc/dcl.h"

static const uint8_t mdkeylen[] = { 16, 20, 28, 32, 48, 64 };

/**
 * cnstr_seq_jobdesc() - Construct simple sequence job descriptor
 * Returns: 0 (for now)
 *
 * @jobdesc - pointer to a buffer to build the target job descriptor
 *            within
 * @jobdescsz - size of target job descriptor buffer
 * @shrdesc - pointer to pre-existing shared descriptor to use with
 *            this job
 * @shrdescsz - size of pre-existing shared descriptor
 * @inbuf - pointer to input frame
 * @insize - size of input frame
 * @outbuf - pointer to output frame
 * @outsize - size of output frame
 *
 * Constructs a simple job descriptor that contains 3 references:
 *   (1) A shared descriptor to do the work. This is normally assumed
 *       to be some sort of a protocol sharedesc, but can be any sharedesc.
 *   (2) A packet/frame for input data
 *   (3) A packet/frame for output data
 *
 * The created descriptor is always a simple reverse-order descriptor,
 * and has no provisions for other content specifications.
 **/
int cnstr_seq_jobdesc(uint32_t *jobdesc, uint16_t *jobdescsz,
		      uint32_t *shrdesc, uint16_t shrdescsz,
		      void *inbuf, uint32_t insize,
		      void *outbuf, uint32_t outsize)
{
	uint32_t *next;

	/*
	 * Basic structure is
	 * - header (assume sharing, reverse order)
	 * - sharedesc physical address
	 * - SEQ_OUT_PTR
	 * - SEQ_IN_PTR
	 */

	/* Make running pointer past where header will go */
	next = jobdesc;
	next++;

	/* Insert sharedesc */
	*next++ = (uint32_t) shrdesc;

	/* Sequence pointers */
	next = cmd_insert_seq_out_ptr(next, outbuf, outsize, PTR_DIRECT);
	next = cmd_insert_seq_in_ptr(next, inbuf, insize, PTR_DIRECT);

	/* Now update header */
	*jobdescsz = next - jobdesc;	/* add 1 to include header */
	cmd_insert_hdr(jobdesc, shrdescsz, *jobdescsz, SHR_SERIAL,
		       SHRNXT_SHARED, ORDER_REVERSE, DESC_STD);

	return 0;
}
EXPORT_SYMBOL(cnstr_seq_jobdesc);

/**
 * Construct a blockcipher request as a single job
 *
 * @descbuf - pointer to buffer for descriptor construction
 * @bufsz - size of constructed descriptor (as output)
 * @data_in - input message
 * @data_out - output message
 * @datasz - size of message
 * @key - cipher key
 * @keylen - size of cipher key
 * @iv - cipher IV
 * @ivlen - size of cipher IV
 * @dir - DIR_ENCRYPT or DIR_DECRYPT
 * @cipher - algorithm from OP_ALG_ALGSEL_
 * @clear - clear descriptor buffer before construction
 **/
int cnstr_jobdesc_blkcipher_cbc(uint32_t *descbuf, uint16_t *bufsz,
				uint8_t *data_in, uint8_t *data_out,
				uint32_t datasz,
				uint8_t *key, uint32_t keylen,
				uint8_t *iv, uint32_t ivlen,
				enum algdir dir, uint32_t cipher, uint8_t clear)
{
	uint32_t *start;
	uint16_t startidx, endidx;
	uint32_t mval;

	start = descbuf++;	/* save start for eventual header write */

	if (!descbuf)
		return -1;

	if (clear)
		memset(start, 0, (*bufsz * sizeof(uint32_t)));

	startidx = descbuf - start;
	startidx = startidx;
	descbuf = cmd_insert_seq_in_ptr(descbuf, data_in, datasz, PTR_DIRECT);

	descbuf = cmd_insert_seq_out_ptr(descbuf, data_out, datasz, PTR_DIRECT);

	descbuf = cmd_insert_load(descbuf, iv, LDST_CLASS_1_CCB,
				  0, LDST_SRCDST_BYTE_CONTEXT, 0, (ivlen >> 3),
				  ITEM_REFERENCE);

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_REFERENCE,
				 ITEM_CLASS1);

	mval = 0;
	descbuf = cmd_insert_math(descbuf, MATH_FUN_SUB, MATH_SRC0_SEQINLEN,
				  MATH_SRC1_IMM, MATH_DEST_VARSEQINLEN,
				  4, 0, 0, 0, &mval);

	descbuf = cmd_insert_math(descbuf, MATH_FUN_ADD, MATH_SRC0_SEQINLEN,
				  MATH_SRC1_IMM, MATH_DEST_VARSEQOUTLEN,
				  4, 0, 0, 0, &mval);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG, cipher,
				    OP_ALG_AAI_CBC, MDSTATE_COMPLETE,
				    ICV_CHECK_OFF, dir);

	descbuf = cmd_insert_seq_fifo_load(descbuf, LDST_CLASS_1_CCB,
					   FIFOLDST_VLF,
					   (FIFOLD_TYPE_MSG |
					    FIFOLD_TYPE_LAST1), 0);

	descbuf = cmd_insert_seq_fifo_store(descbuf, LDST_CLASS_1_CCB,
					    FIFOLDST_VLF,
					    FIFOST_TYPE_MESSAGE_DATA, 0);

	/* Now update the header with size/offsets */
	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsz = endidx;

	return 0;
}
EXPORT_SYMBOL(cnstr_jobdesc_blkcipher_cbc);


/**
 * Generate an MDHA split key - cnstr_jobdesc_mdsplitkey()
 *
 * @descbuf - pointer to buffer to hold constructed descriptor
 *
 * @bufsiz - pointer to size of descriptor once constructed
 *
 * @key - HMAC key to generate ipad/opad from. Size is determined
 *        by cipher:
 *	  - OP_ALG_ALGSEL_MD5    = 16
 *	  - OP_ALG_ALGSEL_SHA1   = 20
 *	  - OP_ALG_ALGSEL_SHA224 = 28 (broken)
 *	  - OP_ALG_ALGSEL_SHA256 = 32
 *	  - OP_ALG_ALGSEL_SHA384 = 48 (broken)
 *	  - OP_ALG_ALGSEL_SHA512 = 64
 *
 * @cipher - HMAC algorithm selection, one of OP_ALG_ALGSEL_
 *
 * @padbuf - buffer to store generated ipad/opad. Should be 2x
 *           the HMAC keysize for chosen cipher rounded up to the
 *           nearest 16-byte boundary (16 bytes = AES blocksize)
 **/
int cnstr_jobdesc_mdsplitkey(uint32_t *descbuf, uint16_t *bufsize,
			     uint8_t *key, uint32_t cipher, uint8_t *padbuf)
{
	uint32_t *start;
	uint16_t startidx, endidx;
	uint8_t keylen, storelen;

	start = descbuf++;
	startidx = descbuf - start;
	startidx = startidx;

	/* Pick key length from cipher submask as an enum */
	keylen = mdkeylen[(cipher & OP_ALG_ALGSEL_SUBMASK) >>
			  OP_ALG_ALGSEL_SHIFT];

	storelen = keylen * 2;

	/* Load the HMAC key */
	descbuf = cmd_insert_key(descbuf, key, keylen * 8, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_REFERENCE,
				 ITEM_CLASS2);

	/*
	 * Select HMAC op with init only, this sets up key unroll
	 * Have DECRYPT selected here, although MDHA doesn't care
	 */
	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS2_ALG, cipher,
				    OP_ALG_AAI_HMAC, MDSTATE_INIT,
				    ICV_CHECK_OFF, DIR_DECRYPT);

	/* FIFO load of 0 to kickstart MDHA (this will generate pads) */
	descbuf = cmd_insert_fifo_load(descbuf, 0, 0, LDST_CLASS_2_CCB,
				       0, FIFOLD_IMM, 0,
				       (FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST2));

	/* Wait for store to complete before proceeding */
	/* This is a tapeout1 dependency */
	descbuf = cmd_insert_jump(descbuf, JUMP_TYPE_LOCAL, CLASS_2,
				  JUMP_TEST_ALL, 0, 1, NULL);

	/* Now store the split key pair with that specific type */
	descbuf = cmd_insert_fifo_store(descbuf, padbuf, storelen,
					LDST_CLASS_2_CCB, 0, 0, 0,
					FIFOST_TYPE_SPLIT_KEK);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}
EXPORT_SYMBOL(cnstr_jobdesc_mdsplitkey);

/**
 * SNOW/f8 (UEA2) as a job descriptor
 *
 * @descbuf - pointer to descriptor-under-construction buffer
 * @bufsize - points to size to be updated at completion
 * @key - cipher key
 * @keylen - size of key in bits
 * @dir - cipher direction (DIR_ENCRYPT/DIR_DECRYPT)
 * @ctx - points to preformatted f8 context block, containing 32-bit count
 *        (word 0), bearer (word 1 bits 0:5), direction (word 1 bit 6),
 *        ca (word 1 bits 7:16), and cb (word 1 bits 17:31). Refer to the
 *        KFHA section of the block guide for more detail.
 * @in - pointer to input data text
 * @out - pointer to output data text
 * @size - size of data to be processed
 **/
int cnstr_jobdesc_snow_f8(uint32_t *descbuf, uint16_t *bufsize,
			    uint8_t *key, uint32_t keylen,
			    enum algdir dir,  uint32_t *ctx,
			    uint8_t *in, uint8_t *out, uint32_t size)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
				 ITEM_CLASS1);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG,
				    OP_ALG_ALGSEL_SNOW, OP_ALG_AAI_F8,
				    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);

	descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_1_CCB,
				  0, LDST_SRCDST_BYTE_CONTEXT, 0, 8,
				  ITEM_INLINE);


	descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,
				       0, 0, 0,
				       FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,
					0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

/**
 * SNOW/f8 (UEA2) as a job descriptor
 *
 * @descbuf - pointer to descriptor-under-construction buffer
 * @bufsize - points to size to be updated at completion
 * @key - cipher key
 * @keylen - size of key in bits
 * @dir - cipher direction (DIR_ENCRYPT/DIR_DECRYPT)
 * @ctx - points to preformatted f8 context block, containing 32-bit count
 *        (word 0), bearer (word 1 bits 0:5), direction (word 1 bit 6),
 *        ca (word 1 bits 7:16), and cb (word 1 bits 17:31). Refer to the
 *        KFHA section of the block guide for more detail.
 * @in - pointer to input data text
 * @out - pointer to output data text
 * @size - size of data to be processed
 **/
int cnstr_jobdesc_snow_f8_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
			    uint8_t *key, uint32_t keylen,
			    enum algdir dir,  uint32_t *ctx,
			    uint8_t *in, uint8_t *out, uint16_t size)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start; 
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
				 ITEM_CLASS1);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG,
				    OP_ALG_ALGSEL_SNOW, OP_ALG_AAI_F8,
				    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);

	descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_1_CCB,
				  0, LDST_SRCDST_BYTE_CONTEXT, 0, 8,
				  ITEM_INLINE);


	descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,
				       FIFOLDST_SGF, /*FIFOLD_IMM*/0, 0,
				       FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,
					0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

EXPORT_SYMBOL(cnstr_jobdesc_snow_f8);

/**
* SNOW/f9 (UIA2) as a job descriptor
*
* @descbuf - pointer to descriptor-under-construction buffer
* @bufsize - points to size to be updated at completion
* @key - cipher key
* @keylen - size of key in bits
* @dir - cipher direction (DIR_ENCRYPT/DIR_DECRYPT)
* @ctx - points to preformatted f9 context block, containing 32-bit count
*        (word 0), bearer (word 1 bits 0:5), direction (word 1 bit 6),
*        ca (word 1 bits 7:16), and cb (word 1 bits 17:31). Refer to the
*        KFHA section of the block guide for more detail.
* @in - pointer to input data text
* @size - size of data to be processed
* @mac - pointer to output MAC
**/
int cnstr_jobdesc_snow_f9(uint32_t *descbuf, uint16_t *bufsize,
                                                   uint8_t *key, uint32_t keylen,
                                                    enum algdir dir, uint32_t *ctx,
                                                    uint8_t *in, uint16_t size, uint8_t *mac)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
	                                                KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
	                                                ITEM_CLASS2);


	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS2_ALG,
							(0xa0 << OP_ALG_ALGSEL_SHIFT), OP_ALG_AAI_F9, 
	                                                    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);


	descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_2_CCB,
	                                                  0, LDST_SRCDST_BYTE_CONTEXT, 0, 16,
	                                                  ITEM_INLINE);

	descbuf = cmd_insert_fifo_load(descbuf, in, size*8/*254*/, FIFOLD_CLASS_CLASS2, 
						   0, 0, 0, 
						   FIFOLD_TYPE_BITDATA | FIFOLD_TYPE_LAST2); 
/* F9基于bit流进行鉴权， 接口传下来字节，这里转换成bit 但验证用例时是254bit，所以测试时需要用254，而不能是32*8，否则出不来用例上的输出结果 */


	descbuf = cmd_insert_store(descbuf, mac, LDST_CLASS_2_CCB, 
						   0, LDST_SRCDST_BYTE_CONTEXT, 0, 4, /*ITEM_INLINE*/0); 

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
	                       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}


/**
* SNOW/f9 (UIA2) as a job descriptor
*
* @descbuf - pointer to descriptor-under-construction buffer
* @bufsize - points to size to be updated at completion
* @key - cipher key
* @keylen - size of key in bits
* @dir - cipher direction (DIR_ENCRYPT/DIR_DECRYPT)
* @ctx - points to preformatted f9 context block, containing 32-bit count
*        (word 0), bearer (word 1 bits 0:5), direction (word 1 bit 6),
*        ca (word 1 bits 7:16), and cb (word 1 bits 17:31). Refer to the
*        KFHA section of the block guide for more detail.
* @in - pointer to input data text
* @size - size of data to be processed
* @mac - pointer to output MAC
**/
int cnstr_jobdesc_snow_f9_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
                                                   uint8_t *key, uint32_t keylen,
                                                    enum algdir dir, uint32_t *ctx,
                                                    uint8_t *in, uint16_t size, uint8_t *mac)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
	                                                KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
	                                                ITEM_CLASS2);


	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS2_ALG,
							(0xa0 << OP_ALG_ALGSEL_SHIFT), OP_ALG_AAI_F9, 
	                                                    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);


	descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_2_CCB,
	                                                  0, LDST_SRCDST_BYTE_CONTEXT, 0, 16,
	                                                  ITEM_INLINE);

	descbuf = cmd_insert_fifo_load(descbuf, in, size*8/*254*/, FIFOLD_CLASS_CLASS2, 
						   FIFOLDST_SGF, 0, 0, 
						   FIFOLD_TYPE_BITDATA | FIFOLD_TYPE_LASTBOTH); 
/* F9基于bit流进行鉴权， 接口传下来字节，这里转换成bit 但验证用例时是254bit，所以测试时需要用254，而不能是32*8，否则出不来用例上的输出结果 */


	descbuf = cmd_insert_store(descbuf, mac, LDST_CLASS_2_CCB, 
						   0, LDST_SRCDST_BYTE_CONTEXT, 0, 4, /*ITEM_INLINE*/0); 

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
	                       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

EXPORT_SYMBOL(cnstr_jobdesc_snow_f9);

uint32_t *cmd_insert_proto_op_lte(uint32_t *descwd, uint8_t cipheralg,
                                                                     uint8_t authalg, enum protdir dir, enum lte_proto_type prtoto_type)
{              
	*descwd = CMD_OPERATION | (0x43 << OP_PCLID_SHIFT);//page 2721

	switch (dir) {
	case DIR_ENCAP:
	                *descwd |= OP_TYPE_ENCAP_PROTOCOL;
	                break;

	case DIR_DECAP:
	                *descwd |= OP_TYPE_DECAP_PROTOCOL;
	                break;

	default:
	                return 0;
	}

	switch (prtoto_type) {
	case lte_snow_f8_f9:
	                *descwd |= (0x0001 << 0);
	                break;

	case lte_aes:
	                *descwd |= (0x0002 << 0);
	                break;

	default:
	                return 0;
	}

	descwd++;

	return descwd;
}
int32_t cnstr_jobdesc_lte_snow_f8_f9(uint32_t *descbuf, uint16_t *bufsize,
                                                                    struct snow_f8_f9_encap_pdb *pdb,
                                                                    struct cipherparams *cipherdata,
                                                                    struct authparams *authdata, uint8_t dir)
{
    uint32_t *start;
    uint16_t startidx, endidx;


    start = descbuf++;


    /* copy in core of PDB */
    memcpy((uint8_t *)descbuf, (uint8_t *)pdb, sizeof(struct snow_f8_f9_encap_pdb));
    descbuf += sizeof(struct snow_f8_f9_encap_pdb) >> 2;

    startidx = descbuf - start;
	startidx = startidx;

    /*
    * Insert an empty instruction for a shared-JUMP past the keys
    * Update later, once the size of the key block is known
    */
    //keyjump = descbuf++;

    /* Insert AES-CMAC keys */
    descbuf = cmd_insert_key(descbuf, authdata->key, authdata->keylen,
                                                    PTR_DIRECT, KEYDST_KEYREG, KEY_CLEAR,
                                                    ITEM_INLINE, ITEM_CLASS2);

    /* Insert AES-CTR keys */
    descbuf = cmd_insert_key(descbuf, cipherdata->key, cipherdata->keylen,
                                                    PTR_DIRECT, KEYDST_KEYREG, KEY_CLEAR,
                                                    ITEM_INLINE, ITEM_CLASS1);


    /*
    * Key jump can now be written (now that we know the size of the
    * key block). This can now happen anytime before the final
    * sizes are computed.
    */
    //cmd_insert_jump(keyjump, JUMP_TYPE_LOCAL, CLASS_BOTH, JUMP_TEST_ALL,
    //                            JUMP_COND_SHRD | JUMP_COND_SELF, descbuf - keyjump, NULL);

    descbuf = cmd_insert_proto_op_lte(descbuf,
                                                    cipherdata->algtype,
                                                    authdata->algtype,
                                                    dir,
                                                                lte_snow_f8_f9);

    endidx = descbuf - start;
    //cmd_insert_shared_hdr(start, startidx, endidx, CTX_SAVE, SHR_SERIAL);
    cmd_insert_hdr(start, startidx, endidx, SHR_NEVER, SHRNXT_LENGTH,
                           ORDER_FORWARD, DESC_STD);
    *bufsize = endidx;
    return 0;
}


int32_t cnstr_jobdesc_lte_aes(uint32_t *descbuf, uint16_t *bufsize,
	                                struct snow_f8_f9_encap_pdb *pdb,
	                                struct cipherparams *cipherdata,
	                                struct authparams *authdata, uint8_t dir)
{
    uint32_t *start;
    uint16_t startidx, endidx;
	uint32_t pdp_len;

	pdp_len = sizeof(struct snow_f8_f9_encap_pdb);// + 8;

    start = descbuf++;


    /* copy in core of PDB */
    memcpy((uint8_t *)descbuf, (uint8_t *)pdb, pdp_len);
    descbuf += pdp_len >> 2;

    startidx = descbuf - start;
	startidx = startidx;

    /*
    * Insert an empty instruction for a shared-JUMP past the keys
    * Update later, once the size of the key block is known
    */
    //keyjump = descbuf++;

    /* Insert AES-CMAC keys */
    descbuf = cmd_insert_key(descbuf, authdata->key, authdata->keylen,
                                                    PTR_DIRECT, KEYDST_KEYREG, KEY_CLEAR,
                                                    ITEM_INLINE, ITEM_CLASS2);

    /* Insert AES-CTR keys */
    descbuf = cmd_insert_key(descbuf, cipherdata->key, cipherdata->keylen,
                                                    PTR_DIRECT, KEYDST_KEYREG, KEY_CLEAR,
                                                    ITEM_INLINE, ITEM_CLASS1);


    /*
    * Key jump can now be written (now that we know the size of the
    * key block). This can now happen anytime before the final
    * sizes are computed.
    */
    //cmd_insert_jump(keyjump, JUMP_TYPE_LOCAL, CLASS_BOTH, JUMP_TEST_ALL,
    //                            JUMP_COND_SHRD | JUMP_COND_SELF, descbuf - keyjump, NULL);

    descbuf = cmd_insert_proto_op_lte(descbuf,
                                                    cipherdata->algtype,
                                                    authdata->algtype,
                                                    dir,
                                                    lte_aes);

    endidx = descbuf - start;
    //cmd_insert_shared_hdr(start, startidx, endidx, CTX_SAVE, SHR_SERIAL);
    cmd_insert_hdr(start, startidx, endidx, SHR_NEVER, SHRNXT_LENGTH,
                           ORDER_FORWARD, DESC_STD);
    *bufsize = endidx;
    return 0;
}



int cnstr_jobdesc_aes_ctr(uint32_t *descbuf, uint16_t *bufsize,
						    uint8_t *key, uint32_t keylen,
						    enum algdir dir,  uint32_t *ctx,
						    uint8_t *in, uint8_t *out, uint16_t size)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
				 ITEM_CLASS1);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG,
				    OP_ALG_ALGSEL_AES, OP_ALG_AAI_CTR_MOD128,
				    //OP_ALG_ALGSEL_SNOW, OP_ALG_AAI_F8,
				    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);

	descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_1_CCB,
				  0, LDST_SRCDST_BYTE_CONTEXT, 16, 16,/* offset = 16 */
				  ITEM_INLINE);


	descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,
				       0, 0, 0,
				       FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,
					0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}




int cnstr_jobdesc_aes_ctr_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
						    uint8_t *key, uint32_t keylen,
						    enum algdir dir,  uint32_t *ctx,
						    uint8_t *in, uint8_t *out, uint16_t size)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
				 ITEM_CLASS1);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG,
				    OP_ALG_ALGSEL_AES, OP_ALG_AAI_CTR_MOD128,
				    //OP_ALG_ALGSEL_SNOW, OP_ALG_AAI_F8,
				    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);

	descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_1_CCB,
				  0, LDST_SRCDST_BYTE_CONTEXT, 16, 16,/* offset = 16 */
				  ITEM_INLINE);


	descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,
				       FIFOLDST_SGF, 0, 0,
				       FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,
					0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

int cnstr_jobdesc_aes_cmac(uint32_t *descbuf, uint16_t *bufsize,
		uint8_t *key, uint32_t keylen,
		enum algdir dir,
		uint8_t *in, uint16_t size, uint8_t *mac)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
			KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
			ITEM_CLASS1);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG,
			OP_ALG_ALGSEL_AES, OP_ALG_AAI_CMAC,
			MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);

	descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,
			0, 0, 0,
			//FIFOLD_TYPE_BITDATA | FIFOLD_TYPE_LAST1);
			FIFOLD_TYPE_MSG| FIFOLD_TYPE_LAST1);

	descbuf = cmd_insert_store(descbuf, mac, LDST_CLASS_1_CCB, 
			0, LDST_SRCDST_BYTE_CONTEXT, 0, 4, 0);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
			ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

int cnstr_jobdesc_aes_cmac_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
						    uint8_t *key, uint32_t keylen,
						    enum algdir dir,
						    uint8_t *in, uint16_t size, uint8_t *mac)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	descbuf = cmd_insert_key(descbuf, key, keylen, PTR_DIRECT,
				 KEYDST_KEYREG, KEY_CLEAR, ITEM_INLINE,
				 ITEM_CLASS1);

	descbuf = cmd_insert_alg_op(descbuf, OP_TYPE_CLASS1_ALG,
				    OP_ALG_ALGSEL_AES, OP_ALG_AAI_CMAC,
				    MDSTATE_COMPLETE, ICV_CHECK_OFF, dir);

	//descbuf = cmd_insert_load(descbuf, ctx, LDST_CLASS_1_CCB,
		//		  0, LDST_SRCDST_BYTE_CONTEXT, 0, 16,
			//	  ITEM_INLINE);

	//memcpy(in - 8, (uint8_t*)ctx, 8);

	//in = in - 8;

	//pktwr_dump("\n####in buffer", (u8*)in, 40);


	descbuf = cmd_insert_fifo_load(descbuf, in, size*8, LDST_CLASS_1_CCB,
				       FIFOLDST_SGF, 0, 0,
				       FIFOLD_TYPE_BITDATA | FIFOLD_TYPE_LASTBOTH);

	//descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,
	//				0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);

	descbuf = cmd_insert_store(descbuf, mac, LDST_CLASS_1_CCB, 
						   0, LDST_SRCDST_BYTE_CONTEXT, 0, 4, /*ITEM_INLINE*/0);

	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
		       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

int cnstr_jobdesc_sec_copy(uint32_t *descbuf, uint16_t *bufsize, uint8_t *in, uint8_t *out, uint16_t size)
{
	uint32_t *start;
	uint16_t startidx, endidx;

	start = descbuf++; 
	startidx = descbuf - start;
	startidx = startidx;

	//copy data from infifo to outfifo
	descbuf = cmd_insert_move(descbuf, 1, MOVE_SRC_INFIFO,
	                                                  MOVE_DEST_OUTFIFO, 0, size);

	descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,
	                                                       0, 0, 0,
	                                                       FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);

	descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,
	                                                                0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);


	endidx = descbuf - start;
	cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
	                       ORDER_FORWARD, DESC_STD);

	*bufsize = endidx;

	return 0;
}

int cnstr_jobdesc_sec_copy_sg(uint32_t *descbuf, uint16_t *bufsize, uint8_t *in, uint8_t *out, uint16_t size)
{
    uint32_t *start;
    uint16_t startidx, endidx;
    
    start = descbuf++; 
    startidx = descbuf - start;
	startidx = startidx;
    
    //copy data from infifo to outfifo
    descbuf = cmd_insert_move(descbuf, 1, MOVE_SRC_INFIFO,
                                    MOVE_DEST_OUTFIFO, 0, size);
    
    descbuf = cmd_insert_fifo_load(descbuf, in, size, LDST_CLASS_1_CCB,//should index class1/2
                                         FIFOLDST_SGF, 0, 0,
                                         FIFOLD_TYPE_MSG | FIFOLD_TYPE_LAST1);//need FIFOLD_TYPE_LAST1/2
    
    descbuf = cmd_insert_fifo_store(descbuf, out, size, FIFOLD_CLASS_SKIP,//can skip class
                                          0, 0, 0, FIFOST_TYPE_MESSAGE_DATA);
    
    
    endidx = descbuf - start;
    cmd_insert_hdr(start, 1, endidx, SHR_NEVER, SHRNXT_LENGTH,
         ORDER_FORWARD, DESC_STD);
    
    *bufsize = endidx;
    
    return 0;
}
 



