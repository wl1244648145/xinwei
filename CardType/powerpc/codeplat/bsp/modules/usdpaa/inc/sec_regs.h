/******************************************************************************

 © 1995-2003, 2004, 2005-2010 Freescale Semiconductor, Inc.
 All rights reserved.

 This is proprietary source code of Freescale Semiconductor Inc.,
 and its use is subject to the NetComm Device Drivers EULA.
 The copyright notice above does not evidence any actual or intended
 publication of such source code.

 **************************************************************************/
#ifndef __SEC_REGS_H
#define __SEC_REGS_H




#if defined(__MWERKS__)
#pragma pack(push,1)
#endif /* __MWERKS__ */
#define MEM_MAP_START


/************************/
/* Memory access macros */
/************************/

#define BSP_GET_BIT8(arg)              *(volatile unsigned char *)(&(arg))
#define BSP_GET_BIT16(arg)             *(volatile unsigned short int*)(&(arg))
#define BSP_GET_BIT32(arg)             *(volatile unsigned int*)(&(arg))
#define BSP_GET_BIT64(arg)             *(volatile unsigned long long*)(&(arg))


#define BSP_WRITE_BIT8(arg, data)     *(volatile unsigned char *)(&(arg)) = (data)
#define BSP_WRITE_BIT16(arg, data)    *(volatile unsigned short int*)(&(arg)) = (data)
#define BSP_WRITE_BIT32(arg, data)    *(volatile unsigned int*)(&(arg)) = (data)
#define BSP_WRITE_BIT64(arg, data)    *(volatile unsigned long long*)(&(arg)) = (data)

#define WRITE_UINT8                 BSP_WRITE_BIT8
#define WRITE_UINT16                BSP_WRITE_BIT16
#define WRITE_UINT32                BSP_WRITE_BIT32
#define WRITE_UINT64                BSP_WRITE_BIT64



/*****************************/
/* General Registers (below) */


typedef  struct t_SecGenRegs
{
    volatile unsigned char  reserved0[4]    ; /* Offset+0x0000: Reserved                        */
    volatile unsigned int mcfgr           ; /* Master Configuration Register                  */
    volatile unsigned char  reserved1[8]    ; /* Offset+0x0008: Reserved                        */
    volatile unsigned long long jq0liodnr       ; /* JQ-0 LIODN Register                            */
    volatile unsigned long long jq1liodnr       ; /* JQ-1 LIODN Register                            */
    volatile unsigned long long jq2liodnr       ; /* JQ-2 LIODN Register                            */
    volatile unsigned long long jq3liodnr       ; /* JQ-3 LIODN Register                            */
    volatile unsigned char  reserved2[48]   ; /* Offset+0x0030: Reserved                        */
    volatile unsigned long long rticaliodnr     ; /* RTIC LIODN Register for Block A                */
    volatile unsigned long long rticbliodnr     ; /* RTIC LIODN Register for Block B                */
    volatile unsigned long long rticcliodnr     ; /* RTIC LIODN Register for Block C                */
    volatile unsigned long long rticdliodnr     ; /* RTIC LIODN Register for Block D                */
    volatile unsigned char  reserved3[28]   ; /* Offset+0x0080: Reserved                        */
    volatile unsigned int decoreqr        ; /* DECO Request Register                          */
    volatile unsigned int deco0liodnr     ; /* DECO-0 LIODN Register                          */
    volatile unsigned char  reserved4[4]    ; /* Offset+0x00A4: Reserved                        */
    volatile unsigned int deco1liodnr     ; /* DECO-1 LIODN Register                          */
    volatile unsigned char  reserved5[4]    ; /* Offset+0x00AC: Reserved                        */
    volatile unsigned int deco2liodnr     ; /* DECO-2 LIODN Register                          */
    volatile unsigned char  reserved6[4]    ; /* Offset+0x00B4: Reserved                        */
    volatile unsigned int deco3liodnr     ; /* DECO-3 LIODN Register                          */
    volatile unsigned char  reserved7[4]    ; /* Offset+0x00BC: Reserved                        */
    volatile unsigned int deco4liodnr     ; /* DECO-4 LIODN Register                          */
    volatile unsigned char  reserved8[92]   ; /* Offset+0x00C4: Reserved                        */
    volatile unsigned int decoavlr        ; /* DECO Availability Register                     */
    volatile unsigned int decorstr        ; /* DECO Reset Register                            */
    volatile unsigned char  reserved9[728]  ; /* Offset+0x0128: Reserved                        */
    volatile unsigned int jdkekr[8]       ; /* Job Descriptor Key Encryption Key Register     */
    volatile unsigned int tdkekr[8]       ; /* Trusted Descriptor Key Encryption Key Register */
    volatile unsigned int tdskr[8]        ; /* Trusted Descriptor Signing Key Register        */
    volatile unsigned char  reserved10[128] ; /* Offset+0x0460: Reserved                        */
    volatile unsigned long long sknr            ; /* Secure Key Nonce Register                      */
    volatile unsigned char  reserved11[2584]; /* Offset+0x04E8: Reserved                        */
}
 t_SecGenRegs;


/* Master Configuration Register */
#define MCFGR_SWRST       ((unsigned int)(1)<<31) /* Software Reset                */
#define MCFGR_WDE         ((unsigned int)(1)<<30) /* DECO Watchdog Enable          */
#define MCFGR_WDF         ((unsigned int)(1)<<29) /* DECO Watchdog Timer Expire    */
#define MCFGR_DMA_RST     ((unsigned int)(1)<<28) /* DMA Reset                     */
#define MCFGR_PS          ((unsigned int)(1)<<16) /* Pointer Size                  */
#define MCFGR_AR_CACHE(x) ((unsigned int)(x)<<12) /* AXI Read Cache Control        */
#define MCFGR_AW_CACHE(x) ((unsigned int)(x)<< 8) /* AXI Write Cache Control       */
#define MCFGR_AXI_PIPE(x) ((unsigned int)(x)<< 4) /* AXI Pipeline Depth            */
#define MCFGR_AXI_PRI     ((unsigned int)(1)<< 3) /* AXI Master Interface Priority */
#define MCFGR_BURST       ((unsigned int)(1)<< 0) /* Maximum Burst Size            */


/* JQ LIODN Register */
#define JQLIODNR_LCK        ((unsigned long long)(1)<<63) /* Lock                          */
#define JQLIODNR_AMTD       ((unsigned long long)(1)<<48) /* Allow Make Trusted Descriptor */
#define JQLIODNR_NSLIODN(x) ((unsigned long long)(x)<<16) /* Non-Sequential LIODN          */
#define JQLIODNR_SLIODN(x)  ((unsigned long long)(x)<< 0) /* Sequential LIODN              */


/* RTIC LIODN Register */
#define RTICLIODNR_LCK       ((unsigned long long)(1)<<63) /* Lock             */
#define RTICLIODNR_SLIODN(x) ((unsigned long long)(x)<< 0) /* Sequential LIODN */


/* DECO Request Register */
#define DECOREQR_DEN4 ((unsigned int)(1)<<16) /* The controller asserts this bit when DECO-4/CCB-4 access permission is granted */
#define DECOREQR_DEN3 ((unsigned int)(1)<<15) /* The controller asserts this bit when DECO-3/CCB-3 access permission is granted */
#define DECOREQR_DEN2 ((unsigned int)(1)<<14) /* The controller asserts this bit when DECO-2/CCB-2 access permission is granted */
#define DECOREQR_DEN1 ((unsigned int)(1)<<13) /* The controller asserts this bit when DECO-1/CCB-1 access permission is granted */
#define DECOREQR_DEN0 ((unsigned int)(1)<<12) /* The controller asserts this bit when DECO-0/CCB-0 access permission is granted */
#define DECOREQR_RQD4 ((unsigned int)(1)<< 4) /* This bit is set by software in order to request DECO-4/CCB-4 access permission */
#define DECOREQR_RQD3 ((unsigned int)(1)<< 3) /* This bit is set by software in order to request DECO-3/CCB-3 access permission */
#define DECOREQR_RQD2 ((unsigned int)(1)<< 2) /* This bit is set by software in order to request DECO-2/CCB-2 access permission */
#define DECOREQR_RQD1 ((unsigned int)(1)<< 1) /* This bit is set by software in order to request DECO-1/CCB-1 access permission */
#define DECOREQR_RQD0 ((unsigned int)(1)<< 0) /* This bit is set by software in order to request DECO-0/CCB-0 access permission */


/* DECO LIODN Register */
#define DECOLIODNR_LCK        ((unsigned long long)(1)<<63) /* Lock                 */
#define DECOLIODNR_NSLIODN(x) ((unsigned long long)(x)<<16) /* Non-Sequential LIODN */
#define DECOLIODNR_SLIODN(x)  ((unsigned long long)(x)<< 0) /* Sequential LIODN     */


/* DECO Availability Register */
#define DECOAVLR_NYA4 ((unsigned int)(1)<<4) /* This bit is set by software to start polling for the availability of DECO-4 */
#define DECOAVLR_NYA3 ((unsigned int)(1)<<3) /* This bit is set by software to start polling for the availability of DECO-3 */
#define DECOAVLR_NYA2 ((unsigned int)(1)<<2) /* This bit is set by software to start polling for the availability of DECO-2 */
#define DECOAVLR_NYA1 ((unsigned int)(1)<<1) /* This bit is set by software to start polling for the availability of DECO-1 */
#define DECOAVLR_NYA0 ((unsigned int)(1)<<0) /* This bit is set by software to start polling for the availability of DECO-0 */


/* DECO Reset Register */
#define DECORSTR_RST4 ((unsigned int)(1)<<4) /* Software writes a 1 to this bit to initiate a soft reset of DECO-4 */
#define DECORSTR_RST3 ((unsigned int)(1)<<3) /* Software writes a 1 to this bit to initiate a soft reset of DECO-3 */
#define DECORSTR_RST2 ((unsigned int)(1)<<2) /* Software writes a 1 to this bit to initiate a soft reset of DECO-2 */
#define DECORSTR_RST1 ((unsigned int)(1)<<1) /* Software writes a 1 to this bit to initiate a soft reset of DECO-1 */
#define DECORSTR_RST0 ((unsigned int)(1)<<0) /* Software writes a 1 to this bit to initiate a soft reset of DECO-0 */


/* General Registers (above) */
/*****************************/


/************************/
/* QI Registers (below) */


typedef  struct t_SecQiRegs
{
    volatile unsigned long long ctlr              ; /* Control Register                                     */
    volatile unsigned char  reserved0[4]      ; /* Offset+0x0008: Reserved                              */
    volatile unsigned int star              ; /* Status Register                                      */
    volatile unsigned long long dqcr              ; /* Dequeue Configuration Register                       */
    volatile unsigned long long eqcr              ; /* Enqueue Configuration Register                       */
    volatile unsigned long long lcr               ; /* LIODN Configuration Register                         */
    volatile unsigned char  reserved1[744]    ; /* Offset+0x0028: Reserved                              */
    volatile unsigned long long idpr_phr[15][2]   ; /* Job x ID Pointer Register / Job x PreHeader Register */
    volatile unsigned char  reserved2[16]     ; /* Offset+0x0400: Reserved                              */
    volatile unsigned long long lr_isr[15][2]     ; /* Job x Links Register / Job x Status Register         */
    volatile unsigned char  reserved3[16]     ; /* Offset+0x0500: Reserved                              */
    volatile unsigned long long ifdhr_ifdlr[15][2]; /* Job x Input Frame Descriptor                         */
    volatile unsigned char  reserved4[16]     ; /* Offset+0x0600: Reserved                              */
    volatile unsigned long long ofdhr_ofdlr[15][2]; /* Job x Output Frame Descriptor                        */
    volatile unsigned char  reserved5[16]     ; /* Offset+0x0700: Reserved                              */
    volatile unsigned long long cfdhr_cfdlr[15][2]; /* Job x Compound Frame Descriptor                      */
    volatile unsigned long long jbsstar           ; /* Jobs Status Register                                 */
    volatile unsigned long long spstar            ; /* Subportals Status Register                           */
    volatile unsigned char  reserved6[1776]   ; /* Offset+0x0810: Reserved                              */
}
 t_SecQiRegs;


/* Control Register */
#define CTLR_EPO   ((unsigned long long)(1)<<49) /* Platform Endian Override                            */
#define CTLR_DMBS  ((unsigned long long)(1)<<48) /* Descriptor Message Data Byte Swap                   */
#define CTLR_CDWSO ((unsigned long long)(1)<<47) /* Software-controlled control data doubleword swap    */
#define CTLR_CWSO  ((unsigned long long)(1)<<46) /* Software-controllable control data word swap        */
#define CTLR_CHWSO ((unsigned long long)(1)<<45) /* Software-controllable control data half-word swap   */
#define CTLR_CBSO  ((unsigned long long)(1)<<44) /* Software-controllable control data byte swapping    */
#define CTLR_MDWSO ((unsigned long long)(1)<<43) /* Software-controllable double word swapping          */
#define CTLR_MWSO  ((unsigned long long)(1)<<42) /* Software-controllable word swapping                 */
#define CTLR_MHWSO ((unsigned long long)(1)<<41) /* Software-controllable half-word swapping            */
#define CTLR_MBSO  ((unsigned long long)(1)<<40) /* Software-controllable byte swapping                 */
#define CTLR_CDWSI ((unsigned long long)(1)<<39) /* Software-controlled control data doubleword swap    */
#define CTLR_CWSI  ((unsigned long long)(1)<<38) /* Software-controllable control data word swap        */
#define CTLR_CHWSI ((unsigned long long)(1)<<37) /* Software-controllable control data half-word swap   */
#define CTLR_CBSI  ((unsigned long long)(1)<<36) /* Software-controllable control data byte swapping    */
#define CTLR_MDWSI ((unsigned long long)(1)<<35) /* Software-controllable message data double word swap */
#define CTLR_MWSI  ((unsigned long long)(1)<<34) /* Software-controllable message data word swap        */
#define CTLR_MHWSI ((unsigned long long)(1)<<33) /* Software-controllable message data half-word swap   */
#define CTLR_MBSI  ((unsigned long long)(1)<<32) /* Software-controllable message data byte swap        */
#define CTLR_SOE   ((unsigned long long)(1)<< 2) /* Stop on error                                       */
#define CTLR_STOP  ((unsigned long long)(1)<< 1) /* Stop                                                */
#define CTLR_DQEN  ((unsigned long long)(1)<< 0) /* Dequeue enable                                      */


/* Status Register */
#define STAR_STOPPED(x) (((unsigned int)(x)&0x80000000)>>31) /* Stopped */
#define STAR_ERRORS(x)  (((unsigned int)(x)&0x0000007F)>> 0) /* Errors  */


/* Dequeue Configuration Register */
#define DQCR_SPFCNT(x) ((unsigned long long)(x)<<56) /* Dequeue Command Subportal Frame Count Threshold */
#define DQCR_CBCNT(x)  ((unsigned long long)(x)<<32) /* Dequeue Command Byte Count                      */
#define DQCR_CSRC(x)   ((unsigned long long)(x)<< 8) /* Dequeue Command Source                          */
#define DQCR_FCNT(x)   ((unsigned long long)(x)<< 4) /* Dequeue Command Frame Count                     */
#define DQCR_CVERB(x)  ((unsigned long long)(x)<< 0) /* Dequeue Command Verb                            */


/* Enqueue Configuration Register */
#define EQCR_CFCOLOR(x) ((unsigned long long)(x)<<32) /* Enqueue Command Frame Color */
#define EQCR_CTAG(x)    ((unsigned long long)(x)<< 0) /* Enqueue Command Tag         */


/* LIODN Configuration Register */
#define LCR_NSLOM(x)    ((unsigned long long)(x)<<48) /* Non-Sequential LIODN Offset Mask */
#define LCR_NSLIODNB(x) ((unsigned long long)(x)<<16) /* Non-Sequential LIODN Base        */
#define LCR_SLIODNB(x)  ((unsigned long long)(x)<< 0) /* Sequential LIODN Base            */


/* Job ID Pointer Register */
#define JIDPR_SDPTR(x) (((unsigned long long)(x)&0x0000000FFFFFFFFFLL)>>0) /* Shared Descriptor Pointer */


/* Job PreHeader Register */
#define JPHR_RSLS(x)          (((unsigned long long)(x)&0x8000000000000000LL)>>63) /* Require SEQ LIODNs to be the Same */
#define JPHR_SDLEN(x)         (((unsigned long long)(x)&0x0000003F00000000LL)>>32) /* Shared Descriptor Length          */
#define JPHR_FSGT(x)          (((unsigned long long)(x)&0x0000000020000000LL)>>29) /* Force Scatter/Gather Table        */
#define JPHR_LONG(x)          (((unsigned long long)(x)&0x0000000010000000LL)>>28) /* Long                              */
#define JPHR_OFFSET(x)        (((unsigned long long)(x)&0x000000000C000000LL)>>26) /* Offset                            */
#define JPHR_ABS(x)           (((unsigned long long)(x)&0x0000000002000000LL)>>25) /* Absolute                          */
#define JPHR_ADDBUF(x)        (((unsigned long long)(x)&0x0000000001000000LL)>>24) /* Add Buffer                        */
#define JPHR_BUF_POOL_ID(x)   (((unsigned long long)(x)&0x0000000000FF0000LL)>>16) /* Buffer Pool ID                    */
#define JPHR_POOL_BUF_SIZE(x) (((unsigned long long)(x)&0x000000000000FFFFLL)>> 0) /* Pool Buffer Size                  */


/* Job Links Register */
#define JLR_EQOR_ID(x)    (((unsigned long long)(x)&0xFFFFFFFFFF000000LL)>>24) /* Enqueue Order Maintenance ID                */
#define JLR_EQOR_FIRST(x) (((unsigned long long)(x)&0x0000000000100000LL)>>20) /* First Job in Enqueue Order Maintanance List */
#define JLR_EQOR_NEXT(x)  (((unsigned long long)(x)&0x00000000000F0000LL)>>16) /* Next Job in Enqueue Order Maintenance List  */
#define JLR_WAIT_FIRST(x) (((unsigned long long)(x)&0x0000000000000F00LL)>> 8) /* First Job in Wait List                      */
#define JLR_WAIT_LAST(x)  (((unsigned long long)(x)&0x00000000000000F0LL)>> 4) /* Last Job in Wait List                       */
#define JLR_WAIT_NEXT(x)  (((unsigned long long)(x)&0x000000000000000FLL)>> 0) /* Next Job in Wait List                       */


/* Job Status Register */
#define JISR_SPID(x)    (((unsigned long long)(x)&0x0000000F00000000LL)>>32) /* Subportal ID                                    */
#define JISR_DONE(x)    (((unsigned long long)(x)&0x0000000040000000LL)>>30) /* Job Done                                        */
#define JISR_EXEC(x)    (((unsigned long long)(x)&0x0000000020000000LL)>>29) /* Job Executing                                   */
#define JISR_IN_USE(x)  (((unsigned long long)(x)&0x0000000010000000LL)>>28) /* Job Data Buffer In Use                          */
#define JISR_OFTLERR(x) (((unsigned long long)(x)&0x0000000000000040LL)>> 6) /* Output Frame Too Large Error                    */
#define JISR_CFWRERR(x) (((unsigned long long)(x)&0x0000000000000020LL)>> 5) /* Compound Frame Scatter/Gather Table Write Error */
#define JISR_BTSERR(x)  (((unsigned long long)(x)&0x0000000000000010LL)>> 4) /* Buffer Too Small Error                          */
#define JISR_BPDERR(x)  (((unsigned long long)(x)&0x0000000000000008LL)>> 3) /* Buffer Pool Depletion Error                     */
#define JISR_OFWRERR(x) (((unsigned long long)(x)&0x0000000000000004LL)>> 2) /* Output Frame Write Error                        */
#define JISR_CFRDERR(x) (((unsigned long long)(x)&0x0000000000000002LL)>> 1) /* Compound Frame Scatter/Gather Table Read Error  */
#define JISR_PHRDERR(x) (((unsigned long long)(x)&0x0000000000000001LL)>> 0) /* PreHeader Read Error                            */


/* Jobs Status Register */
#define JBSSTAR_DONE(x)   (((unsigned long long)(x)&0x0000FFFE00000000LL)>>33) /* Jobs Done               */
#define JBSSTAR_EXEC(x)   (((unsigned long long)(x)&0x00000000FFFE0000LL)>>17) /* Jobs Executing          */
#define JBSSTAR_IN_USE(x) (((unsigned long long)(x)&0x000000000000FFFELL)>> 1) /* Job Data Buffers In Use */


/* Subportals Status Register */
#define SPSTAR_SPFCNT4(x)  (((unsigned long long)(x)&0x000F000000000000LL)>>48) /* Subportal 4 Frame Count          */
#define SPSTAR_SPFCNT3(x)  (((unsigned long long)(x)&0x0000F00000000000LL)>>44) /* Subportal 3 Frame Count          */
#define SPSTAR_SPFCNT2(x)  (((unsigned long long)(x)&0x00000F0000000000LL)>>40) /* Subportal 2 Frame Count          */
#define SPSTAR_SPFCNT1(x)  (((unsigned long long)(x)&0x000000F000000000LL)>>36) /* Subportal 1 Frame Count          */
#define SPSTAR_SPFCNT0(x)  (((unsigned long long)(x)&0x0000000F00000000LL)>>32) /* Subportal 0 Frame Count          */
#define SPSTAR_SPDRDUE4(x) (((unsigned long long)(x)&0x0000000000010000LL)>>16) /* Subportal 4 Dequeue Response Due */
#define SPSTAR_SPDRDUE3(x) (((unsigned long long)(x)&0x0000000000001000LL)>>12) /* Subportal 3 Dequeue Response Due */
#define SPSTAR_SPDRDUE2(x) (((unsigned long long)(x)&0x0000000000000100LL)>> 8) /* Subportal 2 Dequeue Response Due */
#define SPSTAR_SPDRDUE1(x) (((unsigned long long)(x)&0x0000000000000010LL)>> 4) /* Subportal 1 Dequeue Response Due */
#define SPSTAR_SPDRDUE0(x) (((unsigned long long)(x)&0x0000000000000001LL)>> 0) /* Subportal 0 Dequeue Response Due */


/* QI Registers (above) */
/************************/


/************************/
/* JQ Registers (below) */


typedef  struct t_SecJqRegs
{
    volatile unsigned long long irbar           ; /* Input Ring Base Address Register    */
    volatile unsigned char  reserved0[4]    ; /* Offset+0x0008: Reserved             */
    volatile unsigned int irsr            ; /* Input Ring Size Register            */
    volatile unsigned char  reserved1[4]    ; /* Offset+0x0010: Reserved             */
    volatile unsigned int irsar           ; /* Input Ring Slots Available Register */
    volatile unsigned char  reserved2[4]    ; /* Offset+0x0018: Reserved             */
    volatile unsigned int irjar           ; /* Input Ring Jobs Added Register      */
    volatile unsigned long long orbar           ; /* Output Ring Base Address Register   */
    volatile unsigned char  reserved3[4]    ; /* Offset+0x0028: Reserved             */
    volatile unsigned int orsr            ; /* Output Ring Size Register           */
    volatile unsigned char  reserved4[4]    ; /* Offset+0x0030: Reserved             */
    volatile unsigned int orjrr           ; /* Output Ring Jobs Removed Register   */
    volatile unsigned char  reserved5[4]    ; /* Offset+0x0038: Reserved             */
    volatile unsigned int orsfr           ; /* Output Ring Slots Full Register     */
    volatile unsigned char  reserved6[4]    ; /* Offset+0x0040: Reserved             */
    volatile unsigned int ostar           ; /* Output Status Register              */
    volatile unsigned char  reserved7[4]    ; /* Offset+0x0048: Reserved             */
    volatile unsigned int istar           ; /* Interrupt Status Register           */
    volatile unsigned long long cfgr            ; /* Configuration Register              */
    volatile unsigned char  reserved8[4]    ; /* Offset+0x0058: Reserved             */
    volatile unsigned int irrir           ; /* Input Ring Read Index Register      */
    volatile unsigned char  reserved9[4]    ; /* Offset+0x0060: Reserved             */
    volatile unsigned int orwir           ; /* Output Ring Write Index Register    */
    volatile unsigned char  reserved10[4]   ; /* Offset+0x0068: Reserved             */
    volatile unsigned int cmdr            ; /* Command Register                    */
    volatile unsigned char  reserved11[3728]; /* Offset+0x0070: Reserved             */
}
 t_SecJqRegs;


/* Output Status Register */
#define OSTAR_SSRC(x)            (((unsigned int)(x)&0xF0000000)>>28) /* Status source (None, CCB, JHU, DECO, JQ, JHCC) */
#define OSTAR_CCB_JMP(x)         (((unsigned int)(x)&0x08000000)>>24) /* CCB:  The descriptor made a jump               */
#define OSTAR_CCB_DESC_INDEX(x)  (((unsigned int)(x)&0x0000FF00)>> 8) /* CCB:  Index into the failing descriptor        */
#define OSTAR_CCB_ALGORITHM(x)   (((unsigned int)(x)&0x000000F0)>> 4) /* CCB:  Algorithm that generated the error       */
#define OSTAR_CCB_DESC_ERROR(x)  (((unsigned int)(x)&0x0000000F)>> 0) /* CCB:  Descriptor error                         */
#define OSTAR_JHU_JMP(x)         (((unsigned int)(x)&0x08000000)>>24) /* JHU:  The descriptor made a jump               */
#define OSTAR_JHU_DESC_INDEX(x)  (((unsigned int)(x)&0x0000FF00)>> 8) /* JHU:  Index into the failing descriptor        */
#define OSTAR_JHU_STATUS(x)      (((unsigned int)(x)&0x000000FF)>> 0) /* JHU:  User status field from Jump command      */
#define OSTAR_DECO_JMP(x)        (((unsigned int)(x)&0x08000000)>>24) /* DECO: The descriptor made a jump               */
#define OSTAR_DECO_DESC_INDEX(x) (((unsigned int)(x)&0x0000FF00)>> 8) /* DECO: Index into the failing descriptor        */
#define OSTAR_DECO_DESC_ERROR(x) (((unsigned int)(x)&0x000000FF)>> 0) /* DECO: Descriptor error                         */
#define OSTAR_JQ_NADDR(x)        (((unsigned int)(x)&0x00000700)>> 8) /* JQ:   Number of descriptor addresses requested */
#define OSTAR_JQ_DESC_ERROR(x)   (((unsigned int)(x)&0x000000FF)>> 0) /* JQ:   Descriptor error                         */
#define OSTAR_JHT_JMP(x)         (((unsigned int)(x)&0x08000000)>>24) /* JHT:  The descriptor made a jump               */
#define OSTAR_JHT_DESC_INDEX(x)  (((unsigned int)(x)&0x0000FF00)>> 8) /* JHT:  Index into the failing descriptor        */
#define OSTAR_JHT_COND_CODES(x)  (((unsigned int)(x)&0x000000FF)>> 0) /* JHT:  Condition codes field from Jump command  */


/* Interrupt Status Register */
#define ISTAR_ERR_ORWI(x) (((unsigned int)(x)&0x3FFF0000)>>16) /* Output Ring Write Index Error */
#define ISTAR_ERR_TYPE(x) (((unsigned int)(x)&0x00000F00)>> 8) /* Error Type                    */
#define ISTAR_HALTED(x)   (((unsigned int)(x)&0x00000008)>> 3) /* Job Queue Halted              */
#define ISTAR_HALT(x)     (((unsigned int)(x)&0x00000004)>> 2) /* Halt the Job Queue            */
#define ISTAR_JQE(x)      (((unsigned int)(x)&0x00000002)>> 1) /* Job Queue Error               */
#define ISTAR_JQI(x)      (((unsigned int)(x)&0x00000001)>> 0) /* Job Queue Interrupt           */


/* Configuration Register */
#define CFGR_EPO      ((unsigned long long)(1)<<49) /* Platform Endian Override                            */
#define CFGR_DMBS     ((unsigned long long)(1)<<48) /* Descriptor Message Data Byte Swap                   */
#define CFGR_CDWSO    ((unsigned long long)(1)<<47) /* Software-controlled control data doubleword swap    */
#define CFGR_CWSO     ((unsigned long long)(1)<<46) /* Software-controllable control data word swap        */
#define CFGR_CHWSO    ((unsigned long long)(1)<<45) /* Software-controllable control data half-word swap   */
#define CFGR_CBSO     ((unsigned long long)(1)<<44) /* Software-controllable control data byte swapping    */
#define CFGR_MDWSO    ((unsigned long long)(1)<<43) /* Software-controllable double word swapping          */
#define CFGR_MWSO     ((unsigned long long)(1)<<42) /* Software-controllable word swapping                 */
#define CFGR_MHWSO    ((unsigned long long)(1)<<41) /* Software-controllable half-word swapping            */
#define CFGR_MBSO     ((unsigned long long)(1)<<40) /* Software-controllable byte swapping                 */
#define CFGR_CDWSI    ((unsigned long long)(1)<<39) /* Software-controlled control data doubleword swap    */
#define CFGR_CWSI     ((unsigned long long)(1)<<38) /* Software-controllable control data word swap        */
#define CFGR_CHWSI    ((unsigned long long)(1)<<37) /* Software-controllable control data half-word swap   */
#define CFGR_CBSI     ((unsigned long long)(1)<<36) /* Software-controllable control data byte swapping    */
#define CFGR_MDWSI    ((unsigned long long)(1)<<35) /* Software-controllable message data double word swap */
#define CFGR_MWSI     ((unsigned long long)(1)<<34) /* Software-controllable message data word swap        */
#define CFGR_MHWSI    ((unsigned long long)(1)<<33) /* Software-controllable message data half-word swap   */
#define CFGR_MBSI     ((unsigned long long)(1)<<32) /* Software-controllable message data byte swap        */
#define CFGR_ICTT(x)  ((unsigned long long)(x)<<16) /* Interrupt Coalescing Timer Threshold                */
#define CFGR_ICDCT(x) ((unsigned long long)(x)<< 8) /* Interrupt Coalescing Descriptor Count Threshold     */
#define CFGR_ICEN     ((unsigned long long)(1)<< 1) /* Interrupt Coalescing Enable                         */
#define CFGR_IMSK     ((unsigned long long)(1)<< 0) /* Interrupt Mask                                      */


/* Command Register */
#define CMDR_RESET ((unsigned int)(1)<<0) /* Reset */


/* JQ Registers (above) */
/************************/


/**************************/
/* RTIC Registers (below) */


typedef  struct t_SecRticRegs
{
    volatile unsigned char  reserved0[4100] ; /* Offset+0x0000: Reserved                          */
    volatile unsigned int star            ; /* Status Register                                  */
    volatile unsigned char  reserved1[4]    ; /* Offset+0x1008: Reserved                          */
    volatile unsigned int cmdr            ; /* Command Register                                 */
    volatile unsigned char  reserved2[4]    ; /* Offset+0x1010: Reserved                          */
    volatile unsigned int ctlr            ; /* Control Register                                 */
    volatile unsigned char  reserved3[4]    ; /* Offset+0x1018: Reserved                          */
    volatile unsigned int thrr            ; /* Throttle Register                                */
    volatile unsigned char  reserved4[8]    ; /* Offset+0x1020: Reserved                          */
    volatile unsigned long long wdogr           ; /* Watchdog Timer Register                          */
    volatile unsigned char  reserved5[4]    ; /* Offset+0x1030: Reserved                          */
    volatile unsigned int endr            ; /* Endian Register                                  */
    volatile unsigned char  reserved6[200]  ; /* Offset+0x1038: Reserved                          */
    volatile unsigned long long maar0           ; /* Memory Block A Address Register 0                */
    volatile unsigned char  reserved7[4]    ; /* Offset+0x1108: Reserved                          */
    volatile unsigned int malr0           ; /* Memory Block A Length Register 0                 */
    volatile unsigned long long maar1           ; /* Memory Block A Address Register 1                */
    volatile unsigned char  reserved8[4]    ; /* Offset+0x1118: Reserved                          */
    volatile unsigned int malr1           ; /* Memory Block A Length Register 1                 */
    volatile unsigned long long mbar0           ; /* Memory Block B Address Register 0                */
    volatile unsigned char  reserved9[4]    ; /* Offset+0x1128: Reserved                          */
    volatile unsigned int mblr0           ; /* Memory Block B Length Register 0                 */
    volatile unsigned long long mbar1           ; /* Memory Block B Address Register 1                */
    volatile unsigned char  reserved10[4]   ; /* Offset+0x1138: Reserved                          */
    volatile unsigned int mblr1           ; /* Memory Block B Length Register 1                 */
    volatile unsigned long long mcar0           ; /* Memory Block C Address Register 0                */
    volatile unsigned char  reserved11[4]   ; /* Offset+0x1148: Reserved                          */
    volatile unsigned int mclr0           ; /* Memory Block C Length Register 0                 */
    volatile unsigned long long mcar1           ; /* Memory Block C Address Register 1                */
    volatile unsigned char  reserved12[4]   ; /* Offset+0x1158: Reserved                          */
    volatile unsigned int mclr1           ; /* Memory Block C Length Register 1                 */
    volatile unsigned long long mdar0           ; /* Memory Block D Address Register 0                */
    volatile unsigned char  reserved13[4]   ; /* Offset+0x1168: Reserved                          */
    volatile unsigned int mdlr0           ; /* Memory Block D Length Register 0                 */
    volatile unsigned long long mdar1           ; /* Memory Block D Address Register 1                */
    volatile unsigned char  reserved14[4]   ; /* Offset+0x1178: Reserved                          */
    volatile unsigned int mdlr1           ; /* Memory Block D Length Register 1                 */
    volatile unsigned char  reserved15[128] ; /* Offset+0x1180: Reserved                          */
    volatile unsigned int amdb[32]        ; /* Memory Block A Hash Result, Big Endian Format    */
    volatile unsigned int amdl[32]        ; /* Memory Block A Hash Result, Little Endian Format */
    volatile unsigned int bmdb[32]        ; /* Memory Block B Hash Result, Big Endian Format    */
    volatile unsigned int bmdl[32]        ; /* Memory Block B Hash Result, Little Endian Format */
    volatile unsigned int cmdb[32]        ; /* Memory Block C Hash Result, Big Endian Format    */
    volatile unsigned int cmdl[32]        ; /* Memory Block C Hash Result, Little Endian Format */
    volatile unsigned int dmdb[32]        ; /* Memory Block D Hash Result, Big Endian Format    */
    volatile unsigned int dmdl[32]        ; /* Memory Block D Hash Result, Little Endian Format */
    volatile unsigned char  reserved16[2304]; /* Offset+0x1600: Reserved                          */
}
 t_SecRticRegs;


/* Status Register */
#define STAR_CS  0x06000000 /* Current State             */
#define STAR_RTD 0x00080000 /* Run Time Blocks Disabled  */
#define STAR_HOD 0x00040000 /* Hash Once Blocks Disabled */
#define STAR_ABH 0x00020000 /* All Blocks Hashed         */
#define STAR_WE  0x00010000 /* Watchdog Error            */
#define STAR_AE  0x00000F00 /* Address Error             */
#define STAR_MIS 0x000000F0 /* Memory Integrity Status   */
#define STAR_HE  0x00000008 /* Hashing Error             */
#define STAR_SV  0x00000004 /* Security Violation        */


/* Command Register */
#define CMDR_RTD  0x00000008 /* Run Time Disable */
#define CMDR_RTC  0x00000004 /* Run time check   */
#define CMDR_HO   0x00000002 /* Hash once        */
#define CMDR_CINT 0x00000001 /* Clear Interrupt  */


/* Control Register */
#define CTLR_RALG  0x000F0000 /* Algorithm Select        */
#define CTLR_RTMU  0x0000F000 /* Run Time Memory Unlock  */
#define CTLR_RTME  0x00000F00 /* Run Time Memory Enable  */
#define CTLR_HOME  0x000000F0 /* Hash Once Memory Enable */
#define CTLR_RREQS 0x0000000E /* Request Size            */
#define CTLR_IE    0x00000001 /* Interrupt Enable        */


/* Endian Register */
#define ENDR_RDWS 0x000F0000 /* Double Word Swap         */
#define ENDR_RWS  0x0000F000 /* Word Swap                */
#define ENDR_RHWS 0x00000F00 /* Half-Word Swap           */
#define ENDR_RBS  0x000000F0 /* Byte Swap                */
#define ENDR_REPO 0x0000000F /* Endian Platform Override */


/* RTIC Registers (above) */
/**************************/


/**************************/
/* DECO Registers (below) */


typedef  struct t_SecDecoRegs
{
    volatile unsigned long long jqctlr         ; /* JQ Control Register         */
    volatile unsigned long long dar            ; /* Descriptor Address Register */
    volatile unsigned long long opstar         ; /* Operation Status Register   */
    volatile unsigned char  reserved0[8]   ; /* Offset+0x0818: Reserved     */
    volatile unsigned long long lsr            ; /* LIODN Status Register       */
    volatile unsigned char  reserved1[24]  ; /* Offset+0x0828: Reserved     */
    volatile unsigned long long mthr0          ; /* Math Register 0             */
    volatile unsigned long long mthr1          ; /* Math Register 1             */
    volatile unsigned long long mthr2          ; /* Math Register 2             */
    volatile unsigned long long mthr3          ; /* Math Register 3             */
    volatile unsigned char  reserved2[32]  ; /* Offset+0x0860: Reserved     */
    volatile unsigned int gtr0[4]        ; /* Gather Table Register 0     */
    volatile unsigned int gtr1[4]        ; /* Gather Table Register 1     */
    volatile unsigned int gtr2[4]        ; /* Gather Table Register 2     */
    volatile unsigned int gtr3[4]        ; /* Gather Table Register 3     */
    volatile unsigned char  reserved3[64]  ; /* Offset+0x08C0: Reserved     */
    volatile unsigned int str0[4]        ; /* Scatter Table Register 0    */
    volatile unsigned int str1[4]        ; /* Scatter Table Register 1    */
    volatile unsigned int str2[4]        ; /* Scatter Table Register 2    */
    volatile unsigned int str3[4]        ; /* Scatter Table Register 3    */
    volatile unsigned char  reserved4[192] ; /* Offset+0x0940: Reserved     */
    volatile unsigned int desb[64]       ; /* Descriptor Buffer           */
    volatile unsigned char  reserved5[1024]; /* Offset+0x0B00: Reserved     */
}
 t_SecDecoRegs;


/* JQ Control Register */
#define JQCTLR_STEP     0x8000000000000000LL /* Step                    */
#define JQCTLR_SING     0x4000000000000000LL /* Single Step Mode        */
#define JQCTLR_WHL      0x2000000000000000LL /* Whole Descriptor        */
#define JQCTLR_FOUR     0x1000000000000000LL /* Four Words              */
#define JQCTLR_ILE      0x0800000000000000LL /* Immediate Little Endian */
#define JQCTLR_SHR_FROM 0x0700000000000000LL /* Share From              */
#define JQCTLR_SRC      0x0000070000000000LL /* Job Source              */
#define JQCTLR_ID       0x0000000100000000LL /* Job ID                  */
#define JQCTLR_CMD      0x00000000FFFFFFFFLL /* Command                 */


/* Operation Status Register */
#define OPSTAR_STATUS_TYPE   0xF000000000000000LL /* Status Type    */
#define OPSTAR_NLJ           0x0800000000000000LL /* Non-Local Jump */
#define OPSTAR_COMMAND_INDEX 0x00003F0000000000LL /* Command Index  */
#define OPSTAR_STATUS        0x000000FF00000000LL /* Math Status    */
#define OPSTAR_OUT_CT        0x00000000FFFFFFFFLL /* Output Count   */


/* LIODN Status Register */
#define LSR_NSLIODN 0x0FFF000000000000LL /* Non-Sequential LIODN */
#define LSR_SLIODN  0x00000FFF00000000LL /* Sequential LIODN     */
#define LSR_TLIODN  0x0000000000000FFFLL /* Trusted LIODN        */


/* DECO Registers (above) */
/**************************/


/*************************/
/* CCB Registers (below) */


typedef  struct t_SecCcbRegs
{
    volatile unsigned int c1mr           ; /* Class 1 Mode Register      */
    volatile unsigned char  reserved0[4]   ; /* Offset+0x0004: Reserved    */
    volatile unsigned int c1ksr          ; /* Class 1 Key Size Register  */
    volatile unsigned char  reserved1[4]   ; /* Offset+0x000C: Reserved    */
    volatile unsigned long long c1dsr          ; /* Class 1 Data Size Register */
    volatile unsigned char  reserved2[4]   ; /* Offset+0x0018: Reserved    */
    volatile unsigned int c1icvsr        ; /* Class 1 ICV Size Register  */
    volatile unsigned char  reserved3[16]  ; /* Offset+0x0020: Reserved    */
    volatile unsigned int cctlr          ; /* CHA Control Register       */
    volatile unsigned char  reserved4[4]   ; /* Offset+0x0034: Reserved    */
    volatile unsigned int ictlr          ; /* Interrupt Control Register */
    volatile unsigned char  reserved5[4]   ; /* Offset+0x003C: Reserved    */
    volatile unsigned int cwr            ; /* Clear Written Register     */
    volatile unsigned char  reserved6[4]   ; /* Offset+0x0044: Reserved    */
    volatile unsigned long long star           ; /* Status Register            */
    volatile unsigned char  reserved7[8]   ; /* Offset+0x0050: Reserved    */
    volatile unsigned int aadszr         ; /* AAD Size Register          */
    volatile unsigned char  reserved8[4]   ; /* Offset+0x005C: Reserved    */
    volatile unsigned int c1ivszrr       ; /* Class 1 IV Size Register   */
    volatile unsigned char  reserved9[32]  ; /* Offset+0x0064: Reserved    */
    volatile unsigned int pkhaszr        ; /* PKHA A Size Register       */
    volatile unsigned char  reserved10[4]  ; /* Offset+0x0088: Reserved    */
    volatile unsigned int pkbszr         ; /* PKHA B Size Register       */
    volatile unsigned char  reserved11[4]  ; /* Offset+0x0090: Reserved    */
    volatile unsigned int pknszr         ; /* PKHA N Size Register       */
    volatile unsigned char  reserved12[4]  ; /* Offset+0x0098: Reserved    */
    volatile unsigned int pkeszr         ; /* PKHA E Size Register       */
    volatile unsigned char  reserved13[96] ; /* Offset+0x00A0: Reserved    */
    volatile unsigned int c1ctxr[16]     ; /* Class 1 Context Register   */
    volatile unsigned char  reserved14[192]; /* Offset+0x0140: Reserved    */
    volatile unsigned int c1keyr[8]      ; /* Class 1 Key Registers      */
    volatile unsigned char  reserved15[480]; /* Offset+0x0220: Reserved    */
    volatile unsigned int c2mr           ; /* Class 2 Mode Register      */
    volatile unsigned char  reserved16[4]  ; /* Offset+0x0404: Reserved    */
    volatile unsigned int c2ksr          ; /* Class 2 Key Size Register  */
    volatile unsigned char  reserved17[4]  ; /* Offset+0x040C: Reserved    */
    volatile unsigned long long c2dsr          ; /* Class 2 Data Size Register */
    volatile unsigned int c2icvszr       ; /* Class 2 ICV Size Register  */
    volatile unsigned char  reserved18[228]; /* Offset+0x041C: Reserved    */
    volatile unsigned int c2ctxr[18]     ; /* Class 2 Context Register   */
    volatile unsigned char  reserved19[184]; /* Offset+0x0548: Reserved    */
    volatile unsigned int c2keyr[32]     ; /* Class 2 Key Registers      */
    volatile unsigned char  reserved20[336]; /* Offset+0x0680: Reserved    */
    volatile unsigned int ififo          ; /* Input Information FIFO     */
    volatile unsigned char  reserved21[12] ; /* Offset+0x07D4: Reserved    */
    volatile unsigned int dfifo          ; /* Input Data FIFO            */
    volatile unsigned char  reserved22[12] ; /* Offset+0x07E4: Reserved    */
    volatile unsigned long long ofifo          ; /* Output Data FIFO           */
    volatile unsigned char  reserved23[8]  ; /* Offset+0x07F8: Reserved    */
}
 t_SecCcbRegs;


/* Class 1 Mode Register */
#define C1MR_ALG      0x00FF0000 /* Algorithm                               */
#define C1MR_AAI      0x00001FF0 /* Additional Algorithm Information        */
#define C1MR_AS       0x0000000C /* Algorithm State                         */
#define C1MR_ICV_TEST 0x00000002 /* ICV Checking / Test AES Fault Detection */
#define C1MR_ENC      0x00000001 /* Encrypt/Decrypt                         */


/* Class 1 Key Size Register */
#define C1KSR_C1KS 0x0000007F /* Class 1 Key Size */


/* Class 1 Data Size Register */
#define C1DSR_NUMBITS 0xE000000000000000 /* Class 1 Data Size Number of Bits */
#define C1DSR_C1CY    0x0000000100000000 /* Class 1 Data Size Carry          */
#define C1DSR_C1DS    0x00000000FFFFFFFF /* Class 1 Data Size                */


/* Class 1 ICV Size Register */
#define C1ICVSR_C1ICVS 0x0000001F /* Class 1 ICV Size */


/* CHA Control Register */
#define CCTLR_UB   0x08000000 /* Unload the Public Key B Memory  */
#define CCTLR_UA   0x04000000 /* Unload the Public Key A Memory  */
#define CCTLR_UN   0x01000000 /* Unload the Public Key N Memory  */
#define CCTLR_UB3  0x00800000 /* Unload the Public Key B3 Memory */
#define CCTLR_UB2  0x00400000 /* Unload the Public Key B2 Memory */
#define CCTLR_UB1  0x00200000 /* Unload the Public Key B1 Memory */
#define CCTLR_UB0  0x00100000 /* Unload the Public Key B0 Memory */
#define CCTLR_UA3  0x00080000 /* Unload the Public Key A3 Memory */
#define CCTLR_UA2  0x00040000 /* Unload the Public Key A2 Memory */
#define CCTLR_UA1  0x00020000 /* Unload the Public Key A1 Memory */
#define CCTLR_UA0  0x00010000 /* Unload the Public Key A0 Memory */
#define CCTLR_SNF9 0x00000400 /* Reset SNOW f9 Block             */
#define CCTLR_RNG  0x00000200 /* Reset Random Number Generator   */
#define CCTLR_CRC  0x00000100 /* Reset CRC Block                 */
#define CCTLR_MD   0x00000080 /* Reset Hashing Block             */
#define CCTLR_PK   0x00000040 /* Reset Public Key Block          */
#define CCTLR_SNF8 0x00000020 /* Reset SNOW f8 Block             */
#define CCTLR_KAS  0x00000010 /* Reset Kasumi                    */
#define CCTLR_RC4  0x00000008 /* Reset RC4                       */
#define CCTLR_DES  0x00000004 /* Reset DES                       */
#define CCTLR_AES  0x00000002 /* Reset AES                       */
#define CCTLR_ALL  0x00000001 /* Reset All Internal CHAs         */


/* Interrupt Control Register */
#define ICTLR_S9EI 0x04000000 /* SNOW-f9 Error Asserted    */
#define ICTLR_RNEI 0x02000000 /* RNG Error Asserted        */
#define ICTLR_CEI  0x01000000 /* CRC Error Asserted        */
#define ICTLR_MEI  0x00800000 /* Hashing Error Asserted    */
#define ICTLR_PEI  0x00400000 /* Public Key Error Asserted */
#define ICTLR_S8EI 0x00200000 /* SNOW-f8 Error Asserted    */
#define ICTLR_KEI  0x00100000 /* Kasumi Error Asserted     */
#define ICTLR_RCEI 0x00080000 /* ARC4 Error Asserted       */
#define ICTLR_DEI  0x00040000 /* DES Error Asserted        */
#define ICTLR_AEI  0x00020000 /* AES Error Asserted        */
#define ICTLR_S9DI 0x00000400 /* SNOW-f9 Done              */
#define ICTLR_RNDI 0x00000200 /* RNG Done                  */
#define ICTLR_CDI  0x00000100 /* CRC Done                  */
#define ICTLR_MDI  0x00000080 /* Hashing Done              */
#define ICTLR_PDI  0x00000040 /* Public Key Done           */
#define ICTLR_S8DI 0x00000020 /* SNOW-f8 Done              */
#define ICTLR_KDI  0x00000010 /* Kasumi Done               */
#define ICTLR_RCDI 0x00000008 /* ARC4 Done                 */
#define ICTLR_DDI  0x00000004 /* DES Done                  */
#define ICTLR_ADI  0x00000002 /* AES Done                  */


/* Clear Written Register */
#define CWR_C2K   0x00400000 /* Clear the Class 2 Key Register        */
#define CWR_C2C   0x00200000 /* Clear the Class 2 Context Register    */
#define CWR_C2DS  0x00040000 /* Clear the Class 2 Data Size Registers */
#define CWR_C2M   0x00010000 /* Clear the Class 2 Mode Register       */
#define CWR_CPKA  0x00008000 /* Clear the PKHA A Size Register        */
#define CWR_CPKN  0x00004000 /* Clear the PKHA N Size Register        */
#define CWR_CPKB  0x00002000 /* Clear the PKHA B Size Register        */
#define CWR_CPKE  0x00001000 /* Clear the PKHA E Size Register        */
#define CWR_C1K   0x00000040 /* Clear the Class 1 Key Register        */
#define CWR_C1C   0x00000020 /* Clear the Class 1 Context Register    */
#define CWR_C1ICV 0x00000008 /* Clear the Class 1 ICV Size Register   */
#define CWR_C1DS  0x00000004 /* Clear the Class 1 Data Size Register  */
#define CWR_C1M   0x00000001 /* Clear the Class 1 Mode Register       */


/* Status Register */
#define STAR_CL2    0xF000000000000000LL /* Class 2 Algorithms           */
#define STAR_ERRID2 0x000F000000000000LL /* Error ID 2                   */
#define STAR_CL1    0x0000F00000000000LL /* Class 1 Algorithms           */
#define STAR_ERRID1 0x0000000F00000000LL /* Error ID 1                   */
#define STAR_PIZ    0x0000000040000000LL /* Public Key Operation is Zero */
#define STAR_GCD    0x0000000020000000LL /* GCD is One                   */
#define STAR_PRM    0x0000000010000000LL /* Public Key is Prime          */
#define STAR_SEI    0x0000000000200000LL /* Class 2 Error Interrupt      */
#define STAR_PEI    0x0000000000100000LL /* Class 1 Error Interrupt      */
#define STAR_SDI    0x0000000000020000LL /* Class 2 Done Interrupt       */
#define STAR_PDI    0x0000000000010000LL /* Class 1 Done Interrupt       */
#define STAR_S9B    0x0000000000000400LL /* SNOW f9 Block Busy           */
#define STAR_RNB    0x0000000000000200LL /* RNG Block Busy               */
#define STAR_CB     0x0000000000000100LL /* CRC Block Busy               */
#define STAR_MB     0x0000000000000800LL /* Hashing Block Busy           */
#define STAR_PB     0x0000000000000400LL /* Public Key Block Busy        */
#define STAR_SB     0x0000000000000200LL /* SNOW Block Busy              */
#define STAR_KB     0x0000000000000100LL /* Kasumi Block Busy            */
#define STAR_RCB    0x0000000000000800LL /* ARC4 Block Busy              */
#define STAR_DB     0x0000000000000400LL /* DES Block Busy               */
#define STAR_AB     0x0000000000000200LL /* AES Block Busy               */


/* AAD Size Register */
#define AADSZR_AASZ 0x0000000F /* AAD Size Mod 16 */


/* Class 1 IV Size Register */
#define C1IVSZR_IVSZ 0x0000000F /* IV Size Mod 16 */


/* PKHA A Size Register */
#define PKHASZR_PKASZ 0x000003FF /* PKHA A Memory Key Size */


/* PKHA B Size Register */
#define PKHBSZR_PKBSZ 0x000003FF /* PKHA B Memory Key Size */


/* PKHA N Size Register */
#define PKHNSZR_PKNSZ 0x000003FF /* PKHA N Memory Key Size */


/* PKHA E Size Register */
#define PKHESZR_PKESZ 0x000003FF /* PKHA E Memory Key Size */


/* Class 2 Mode Register */
#define C2MR_ALG  0x00FF0000 /* Algorithm                        */
#define C2MR_AAI  0x00001FF0 /* Additional Algorithm Information */
#define C2MR_AS   0x0000000C /* Algorithm State                  */
#define C2MR_ICV  0x00000002 /* ICV Checking                     */
#define C2MR_AP   0x00000001 /* Authenticate / Protect           */


/* Class 2 Key Size Register */
#define C2KSR_C2KS 0x000000FF /* Class 2 Key Size */


/* Class 2 Data Size Register */
#define C2DSR_NUMBITS 0xE000000000000000LL /* Class 2 Data Size Number of Bits */
#define C2DSR_C2CY    0x0000000100000000LL /* Class 2 Data Size Carry          */
#define C2DSR_C2DS    0x00000000FFFFFFFFLL /* Class 2 Data Size                */


/* Class 2 ICV Size Register */
#define C2ICVSZR_ICVSZ 0x00000007 /* Class 2 ICV Size Mod 8 */


/* Input Information FIFO */
#define IFIFO_DEST           0xC0000000 /* Destination                            */
#define IFIFO_LC2            0x20000000 /* Last Class 2                           */
#define IFIFO_LC1            0x10000000 /* Last Class 1                           */
#define IFIFO_FC2            0x08000000 /* Flush Class 2                          */
#define IFIFO_FC1            0x04000000 /* Flush Class 1                          */
#define IFIFO_STYPE          0x03000000 /* Source Type                            */
#define IFIFO_DTYPE          0x00F00000 /* Data Type                              */
#define IFIFO_BND            0x00080000 /* Boundary Padding                       */
#define IFIFO_PTYPE          0x00070000 /* Pad Type                               */
#define IFIFO_OC             0x00008000 /* Output FIFO Continuation               */
#define IFIFO_AST            0x00004000 /* Additional Source Type                 */
#define IFIFO_NON_PADDING_DL 0x00000FFF /* Data Length (for non-padding entries)  */
#define IFIFO_PADDING_BM     0x00000080 /* Boundary Minus 1 (for padding entries) */
#define IFIFO_PADDING_PS     0x00000040 /* Pad Snoop (for padding entries)        */
#define IFIFO_PADDING_PL     0x0000007F /* Pad Length (for padding entries)       */


/* CCB Registers (above) */
/*************************/


/****************************/
/* Status Registers (below) */


typedef  struct t_SecStatRegs
{
    volatile unsigned long long req_deq       ; /* Number of Requests Dequeued         */
    volatile unsigned long long ob_enc_req    ; /* Number of Outbound Encrypt Requests */
    volatile unsigned long long ib_dec_req    ; /* Number of Inbound Decrypt Requests  */
    volatile unsigned long long ob_encrypt    ; /* Number of Outbound Bytes Encrypted  */
    volatile unsigned long long ob_protect    ; /* Number of Outbound Bytes Protected  */
    volatile unsigned long long ib_decrypt    ; /* Number of Inbound Bytes Decrypted   */
    volatile unsigned long long ib_validated  ; /* Number of Inbound Bytes Validated   */
    volatile unsigned char  reserved0[104]; /* Offset+0x0038: Reserved             */
    volatile unsigned long long crnr          ; /* CHA Revision Number Register        */
    volatile unsigned long long ctpr          ; /* Compile Time Parameters Register    */
    volatile unsigned char  reserved1[16] ; /* Offset+0x00B0: Reserved             */
    volatile unsigned long long far           ; /* Fault Address Register              */
    volatile unsigned int falr          ; /* Fault Address LIODN Register        */
    volatile unsigned int fadr          ; /* Fault Address Detail Register       */
    volatile unsigned char  reserved2[4]  ; /* Offset+0x00D0: Reserved             */
    volatile unsigned int sstar         ; /* SEC 4.0 Status Register             */
    volatile unsigned char  reserved3[8]  ; /* Offset+0x00D8: Reserved             */
    volatile unsigned int rvidr         ; /* RTIC Version ID Register            */
    volatile unsigned int ccbvidr       ; /* CCB Version ID Register             */
    volatile unsigned long long chavidr       ; /* CHA Version ID Register             */
    volatile unsigned long long chanumr       ; /* CHA Number Register                 */
    volatile unsigned long long secvidr       ; /* SEC 4.0 Version ID Register         */
}
 t_SecStatRegs;


/* CHA Revision Number Register */
#define CRNR_JQRN(x)   (((unsigned long long)(x)&0xF000000000000000LL)>>60) /* JQ Revision Number                      */
#define CRNR_DECORN(x) (((unsigned long long)(x)&0x0F00000000000000LL)>>56) /* DECO Revision Number                    */
#define CRNR_SNW9RN(x) (((unsigned long long)(x)&0x000000F000000000LL)>>36) /* SNOW-f9 Revision Number                 */
#define CRNR_CRCRN(x)  (((unsigned long long)(x)&0x0000000F00000000LL)>>32) /* CRC Module Revision Number              */
#define CRNR_PKRN(x)   (((unsigned long long)(x)&0x00000000F0000000LL)>>28) /* Public Key Module Revision Number       */
#define CRNR_KASRN(x)  (((unsigned long long)(x)&0x000000000F000000LL)>>24) /* Kasumi Module Revision Number           */
#define CRNR_SNW8RN(x) (((unsigned long long)(x)&0x0000000000F00000LL)>>20) /* SNOW-f8 Module Revision Number          */
#define CRNR_RNGRN(x)  (((unsigned long long)(x)&0x00000000000F0000LL)>>16) /* Random Number Generator Revision Number */
#define CRNR_MDRN(x)   (((unsigned long long)(x)&0x000000000000F000LL)>>12) /* Hashing Module Revision Number          */
#define CRNR_ARC4RN(x) (((unsigned long long)(x)&0x0000000000000F00LL)>> 8) /* ARC4 Module Revision Number             */
#define CRNR_DESRN(x)  (((unsigned long long)(x)&0x00000000000000F0LL)>> 4) /* DES Module Revision Number              */
#define CRNR_AESRN(x)  (((unsigned long long)(x)&0x000000000000000FLL)>> 0) /* AES Module Revision Number              */


/* Compile Time Parameters Register */
#define CTPR_AXI_PIPE_DEPTH(x) (((unsigned long long)(x)&0xF000000000000000LL)>>60) /* AXI Pipeline Depth                                                                         */
#define CTPR_AXI_LIODN(x)      (((unsigned long long)(x)&0x0800000000000000LL)>>59) /* Logic to select LIODNs is included                                                         */
#define CTPR_AXI_PRI(x)        (((unsigned long long)(x)&0x0400000000000000LL)>>58) /* Logic for the AXI Master Priority signals is included                                      */
#define CTPR_QI(x)             (((unsigned long long)(x)&0x0200000000000000LL)>>57) /* Queue Manager Interface is included                                                        */
#define CTPR_ACC_CTL(x)        (((unsigned long long)(x)&0x0100000000000000LL)>>56) /* MID-based access control for the IP Bus registers is implemented                           */
#define CTPR_C1C2(x)           (((unsigned long long)(x)&0x0080000000000000LL)>>55) /* Class 2 Key and Context registers are separate from Class 1 Key and Context registers      */
#define CTPR_PC(x)             (((unsigned long long)(x)&0x0020000000000000LL)>>53) /* Performance Counter registers are implemented                                              */
#define CTPR_DECO_WD(x)        (((unsigned long long)(x)&0x0010000000000000LL)>>52) /* DECO Watchdog Counter is implemented                                                       */
#define CTPR_PM_EVT_BUS(x)     (((unsigned long long)(x)&0x0008000000000000LL)>>51) /* Performance Monitor Event Bus is implemented                                               */
#define CTPR_SG8(x)            (((unsigned long long)(x)&0x0004000000000000LL)>>50) /* 8-entry Scatter-Gather Tables are implemented                                              */
#define CTPR_MCFG_PS(x)        (((unsigned long long)(x)&0x0002000000000000LL)>>49) /* Master Configuration Register contains a Pointer Size field                                */
#define CTPR_MCFG_BURST(x)     (((unsigned long long)(x)&0x0001000000000000LL)>>48) /* Master Configuration Register contains an AXI Burst field                                  */
#define CTPR_IP_CLK(x)         (((unsigned long long)(x)&0x0000400000000000LL)>>46) /* Frequency of the IP Bus Slave Clock is one-half the frequency of the system clock          */
#define CTPR_DBL_CRC(x)        (((unsigned long long)(x)&0x0000000000001000LL)>>12) /* Specialized support for 3G Double CRC is implemented                                       */
#define CTPR_P3G_LTE(x)        (((unsigned long long)(x)&0x0000000000000800LL)>>11) /* Specialized support for 3G and LTE protocols is implemented                                */
#define CTPR_RSA(x)            (((unsigned long long)(x)&0x0000000000000400LL)>>10) /* Specialized support for RSA encrypt and decrypt operations is implemented                  */
#define CTPR_MACSEC(x)         (((unsigned long long)(x)&0x0000000000000200LL)>> 9) /* Specialized support for the MACSEC protocol is implemented                                 */
#define CTPR_TLS_PRF(x)        (((unsigned long long)(x)&0x0000000000000100LL)>> 8) /* Specialized support for the TLS protocol pseudo-random function is implemented             */
#define CTPR_SSL_TLS(x)        (((unsigned long long)(x)&0x0000000000000080LL)>> 7) /* Specialized support for the SSL and TLS protocols is implemented                           */
#define CTPR_IKE(x)            (((unsigned long long)(x)&0x0000000000000040LL)>> 6) /* Specialized support for the IKE protocol is implemented                                    */
#define CTPR_IPSEC(x)          (((unsigned long long)(x)&0x0000000000000020LL)>> 5) /* Specialized support for the IPSEC protocol is implemented                                  */
#define CTPR_SRTP(x)           (((unsigned long long)(x)&0x0000000000000010LL)>> 4) /* Specialized support for the SRTP protocol is implemented                                   */
#define CTPR_WIMAX(x)          (((unsigned long long)(x)&0x0000000000000008LL)>> 3) /* Specialized support for the WIMAX protocol is implemented                                  */
#define CTPR_WIFI(x)           (((unsigned long long)(x)&0x0000000000000004LL)>> 2) /* Specialized support for the WIFI protocol is implemented                                   */
#define CTPR_BLOB(x)           (((unsigned long long)(x)&0x0000000000000002LL)>> 1) /* Specialized support for encapsulating and decapsulating cryptographic blobs is implemented */
#define CTPR_KG_DS(x)          (((unsigned long long)(x)&0x0000000000000001LL)>> 0) /* Specialized support for Key Generation and Digital Signatures is implemented               */


/* Fault Address LIODN Register */
#define FALR_FLIODN(x) (((unsigned int)(x)&0x00000FFF)>>0) /* DMA transaction LIODN */


/* Fault Address Detail Register */
#define FADR_FERR(x)  (((unsigned int)(x)&0xC0000000)>>30) /* Fault Error Code              */
#define FADR_DTYP(x)  (((unsigned int)(x)&0x00008000)>>15) /* Data Type                     */
#define FADR_JSRC(x)  (((unsigned int)(x)&0x00007000)>>12) /* Job Source                    */
#define FADR_BLKID(x) (((unsigned int)(x)&0x00000F00)>> 8) /* Block ID                      */
#define FADR_TYP(x)   (((unsigned int)(x)&0x00000080)>> 7) /* AXI Transaction Type          */
#define FADR_FSZ(x)   (((unsigned int)(x)&0x0000007F)>> 0) /* AXI Transaction Transfer Size */


/* SEC 4.0 Status Register */
#define STTAR_PLEND(x) (((unsigned int)(x)&0x00000400)>>10) /* Platform Endianness */
#define STTAR_MOO(x)   (((unsigned int)(x)&0x00000300)>> 8) /* Mode of Operation   */
#define STTAR_BSY(x)   (((unsigned int)(x)&0x00000001)>> 0) /* SEC 4.0 Busy        */


/* RTIC Version ID Register */
#define RVIDR_MD(x)       (((unsigned int)(x)&0x08000000)>>27) /* Memory Block D Available */
#define RVIDR_MC(x)       (((unsigned int)(x)&0x04000000)>>26) /* Memory Block C Available */
#define RVIDR_MB(x)       (((unsigned int)(x)&0x02000000)>>25) /* Memory Block B Available */
#define RVIDR_MA(x)       (((unsigned int)(x)&0x01000000)>>24) /* Memory Block A Available */
#define RVIDR_SHA2_512(x) (((unsigned int)(x)&0x00080000)>>19) /* SHA2-512 Supported       */
#define RVIDR_SHA2_256(x) (((unsigned int)(x)&0x00020000)>>17) /* SHA2-256 Supported       */
#define RVIDR_RMJV(x)     (((unsigned int)(x)&0x0000FF00)>> 8) /* RTIC Major Version       */
#define RVIDR_RMNV(x)     (((unsigned int)(x)&0x000000FF)>> 0) /* RTIC Minor Version       */


/* CCB Version ID Register */
#define CCBVIDR_AMJV(x) (((unsigned int)(x)&0x0000FF00)>>8) /* Accelerator Major Revision Number */
#define CCBVIDR_AMNV(x) (((unsigned int)(x)&0x000000FF)>>0) /* Accelerator Minor Revision Number */


/* CHA Version ID Register */
#define CHAVIDR_JQVID(x)   (((unsigned long long)(x)&0xF000000000000000LL)>>60) /* JQ Revision Number                      */
#define CHAVIDR_DECOVID(x) (((unsigned long long)(x)&0x0F00000000000000LL)>>56) /* DECO Revision Number                    */
#define CHAVIDR_SNW9VID(x) (((unsigned long long)(x)&0x000000F000000000LL)>>36) /* SNOW-f9 Revision Number                 */
#define CHAVIDR_CRCVID(x)  (((unsigned long long)(x)&0x0000000F00000000LL)>>32) /* CRC Module Revision Number              */
#define CHAVIDR_PKVID(x)   (((unsigned long long)(x)&0x00000000F0000000LL)>>28) /* Public Key Module Revision Number       */
#define CHAVIDR_KASVID(x)  (((unsigned long long)(x)&0x000000000F000000LL)>>24) /* Kasumi Module Revision Number           */
#define CHAVIDR_SNW8VID(x) (((unsigned long long)(x)&0x0000000000F00000LL)>>20) /* SNOW-f8 Module Revision Number          */
#define CHAVIDR_RNGVID(x)  (((unsigned long long)(x)&0x00000000000F0000LL)>>16) /* Random Number Generator Revision Number */
#define CHAVIDR_MDVID(x)   (((unsigned long long)(x)&0x000000000000F000LL)>>12) /* Hashing Module Revision Number          */
#define CHAVIDR_ARC4VID(x) (((unsigned long long)(x)&0x0000000000000F00LL)>> 8) /* ARC4 Module Revision Number             */
#define CHAVIDR_DESVID(x)  (((unsigned long long)(x)&0x00000000000000F0LL)>> 4) /* DES Module Revision Number              */
#define CHAVIDR_AESVID(x)  (((unsigned long long)(x)&0x000000000000000FLL)>> 0) /* AES Module Revision Number              */


/* CHA Number Register */
#define CHANUMR_JQNUM(x)   (((unsigned long long)(x)&0xF000000000000000LL)>>60) /* JQ Number of Copies                      */
#define CHANUMR_DECONUM(x) (((unsigned long long)(x)&0x0F00000000000000LL)>>56) /* DECO Number of Copies                    */
#define CHANUMR_SNW9NUM(x) (((unsigned long long)(x)&0x000000F000000000LL)>>36) /* SNOW-f9 Number of Copies                 */
#define CHANUMR_CRCNUM(x)  (((unsigned long long)(x)&0x0000000F00000000LL)>>32) /* CRC Module Number of Copies              */
#define CHANUMR_PKNUM(x)   (((unsigned long long)(x)&0x00000000F0000000LL)>>28) /* Public Key Module Number of Copies       */
#define CHANUMR_KASNUM(x)  (((unsigned long long)(x)&0x000000000F000000LL)>>24) /* Kasumi Module Number of Copies           */
#define CHANUMR_SNW8NUM(x) (((unsigned long long)(x)&0x0000000000F00000LL)>>20) /* SNOW-f8 Module Number of Copies          */
#define CHANUMR_RNGNUM(x)  (((unsigned long long)(x)&0x00000000000F0000LL)>>16) /* Random Number Generator Number of Copies */
#define CHANUMR_MDNUM(x)   (((unsigned long long)(x)&0x000000000000F000LL)>>12) /* Hashing Module Number of Copies          */
#define CHANUMR_ARC4NUM(x) (((unsigned long long)(x)&0x0000000000000F00LL)>> 8) /* ARC4 Module Number of Copies             */
#define CHANUMR_DESNUM(x)  (((unsigned long long)(x)&0x00000000000000F0LL)>> 4) /* DES Module Number of Copies              */
#define CHANUMR_AESNUM(x)  (((unsigned long long)(x)&0x000000000000000FLL)>> 0) /* AES Module Number of Copies              */


/* SEC 4.0 Version ID Register */
#define SECVIDR_IP_ID(x)       (((unsigned long long)(x)&0xFFFF000000000000LL)>>48) /* ID for SEC 4.0                    */
#define SECVIDR_MAJ_REV(x)     (((unsigned long long)(x)&0x0000FF0000000000LL)>>40) /* Major Revision Number for SEC 4.0 */
#define SECVIDR_MIN_REV(x)     (((unsigned long long)(x)&0x000000FF00000000LL)>>32) /* Minor Revision Number for SEC 4.0 */
#define SECVIDR_COMPILE_OPT(x) (((unsigned long long)(x)&0x00000000FF000000LL)>>24) /* Compile Options for SEC 4.0       */
#define SECVIDR_INTG_OPT(x)    (((unsigned long long)(x)&0x0000000000FF0000LL)>>16) /* Integration Options for SEC 4.0   */
#define SECVIDR_ECO_REV(x)     (((unsigned long long)(x)&0x000000000000FF00LL)>> 8) /* ECO Revision Number for SEC 4.0   */
#define SECVIDR_CONFIG_OPT(x)  (((unsigned long long)(x)&0x00000000000000FFLL)>> 0) /* Configuration Options for SEC 4.0 */


/* Status Registers (above) */
/****************************/


/***********************/
/* Memory Maps (below) */


typedef  struct t_SecGenMemMap
{
    t_SecGenRegs  gen ; /* Base+0x0000: General Registers */
    t_SecStatRegs stat; /* Base+0x0F00: Status Registers  */
}
 t_SecGenMemMap;


typedef  struct t_SecQiMemMap
{
    t_SecQiRegs   qi  ; /* Base+0x0000: QI Registers     */
    t_SecStatRegs stat; /* Base+0x0F00: Status Registers */
}
 t_SecQiMemMap;


typedef  struct t_SecJqMemMap
{
    t_SecJqRegs   jq  ; /* Base+0x0000: JQ Registers     */
    t_SecStatRegs stat; /* Base+0x0F00: Status Registers */
}
 t_SecJqMemMap;


typedef  struct t_SecRticMemMap
{
    t_SecRticRegs rtic ; /* Base+0x0000: RTIC Registers   */
    t_SecStatRegs stat5; /* Base+0x0F00: Status Registers */
}
 t_SecRticMemMap;


typedef  struct t_SecDecoCcbMemMap
{
    t_SecCcbRegs  ccb ; /* Base+0x0000: CCB Registers    */
    t_SecDecoRegs deco; /* Base+0x0800: DECO Registers   */
    t_SecStatRegs stat; /* Base+0x0F00: Status Registers */
}
 t_SecDecoCcbMemMap;


/* Memory Maps (above) */
/***********************/


/**********************************/
/* 64-Bit Register Access (below) */


#define SEC_GET_UINT64_HI_LO(arg)                                \
(                                                                \
    ((unsigned long long)GET_UINT32(*((unsigned int*)&(arg)+0))<<32) |         \
    ((unsigned long long)GET_UINT32(*((unsigned int*)&(arg)+1))<< 0)           \
)

#define SEC_GET_UINT64_LO_HI(arg)                                \
(                                                                \
    ((unsigned long long)GET_UINT32(*((unsigned int*)&(arg)+1))<< 0) |         \
    ((unsigned long long)GET_UINT32(*((unsigned int*)&(arg)+0))<<32)           \
)

#define SEC_WRITE_UINT64_HI_LO(arg,data)                         \
{                                                                \
    WRITE_UINT32(*((unsigned int*)&(arg)+0),(unsigned int)((data)>>32)); \
    WRITE_UINT32(*((unsigned int*)&(arg)+1),(unsigned int)((data)>> 0)); \
}

#define SEC_WRITE_UINT64_LO_HI(arg,data)                         \
{                                                                \
    WRITE_UINT32(*((unsigned int*)&(arg)+1),(unsigned int)((data)>> 0)); \
    WRITE_UINT32(*((unsigned int*)&(arg)+0),(unsigned int)((data)>>32)); \
}


/* 64-Bit Register Access (above) */
/**********************************/


#define MEM_MAP_END
#if defined(__MWERKS__)
#pragma pack(pop)
#endif /* __MWERKS__ */


#endif /* __SEC_REGS_H */
