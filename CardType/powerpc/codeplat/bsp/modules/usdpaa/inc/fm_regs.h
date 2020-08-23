#ifndef __HEADER_FM_REGS_H
#define __HEADER_FM_REGS_H
/* 寄存器base */
//#define CCSR_VIRTADDR_BASE                        0x40000000


/* 寄存器第一级 */
#define CCSR_FM1_OFFSET                            0x400000
#define CCSR_FM2_OFFSET                            0x500000

/* 寄存器第二级 */
#define FM_BMIQMIPARSER_OFFSET               0x80000
#define FM_POLICER_OFFSET                         0xC0000
#define FM_KG_OFFSET                                 0xC1000
#define FM_DMA_OFFSET                               0xC2000
#define FM_FPM_OFFSET                             0xC3000
#define FM_PARSERGLOBAL_OFFSET            0xC7000
#define FM_dTSEC_OFFSET                           0xC8000



#define FM_FMPR_RPIMAC                          0xC7844

/* 寄存器第三级 */
#define FM_BMIQMIPARSER_PERPORTSIZE               0x1000

#define BMI_OFFSET                    0x0
#define QMI_OFFSET                    0x400
#define PARSER_OFFSET                 0x800


/* 寄存器第四级 */

#define FMBM_INIT    0x000
#define FMBM_CFG1    0x004
#define FMBM_CFG2    0x008
#define FMBM_IEVR    0x020
#define FMBM_IER     0x024
#define FMBM_IFR     0x028
#define FMBM_GDE     0x100

#define FMBM_RCFG           0x000         /*      Rx Configuration                                             */         
#define FMBM_RST           0x004          /*     Rx Status                                                     */         
#define FMBM_RDA            0x008         /*      Rx DMA attributes R/W All zeros 8.5.3.3.3/-123               */         
#define FMBM_RFP            0x00C         /*      Rx FIFO Parameters R/W 0x03FF_03FF 8.5.3.3.4/-125            */         
#define FMBM_RFED           0x010         /*      Rx Frame End Data R/W All zeros 8.5.3.3.5/-126               */         
#define FMBM_RICP           0x014         /*      Rx Internal Context Parameters R/W All zeros 8.5.3.3.6/-127  */          
#define FMBM_RIM            0x018         /*      Rx Internal Margins R/W All zeros 8.5.3.3.7/-128             */         
#define FMBM_REBM           0x01C         /*      Rx External Buffer Margins R/W 0x0002_0000 8.5.3.3.8/-129    */         
#define FMBM_RFNE						0x020 				/*			Rx Frame Next Engine R/W 0x0044_0000 8.5.3.3.9/-129          */         
#define FMBM_RFCA						0x024 				/*			Rx Frame Attributes. R/W 0x803C_0000 8.5.3.3.10/-130         */         
#define FMBM_RFPNE					0x028 				/*				Rx Frame Parser Next Engine R/W 0x0048_0000 8.5.3.3.11/-1  */           
#define FMBM_RPSO						0x02C 				/*		Rx Parse Start Offset R/W All zeros 8.5.3.3.12/-132            */         
#define FMBM_RPP						0x030 				/*				Rx Policer Profile R/W All zeros 8.5.3.3.13/-132           */         
#define FMBM_RCCB						0x034 				/*			Rx Coarse Classification Base R/W All zeros 8.5.3.3.14/-133  */         
#define FMBM_RETH						0x038 				/*							Rx Excessive Threshold R/W 0x0000_03FF 8.5.3.3.15/-  */            
#define FMBM_RFQID					0x060 				/*				Rx Frame Queue ID R/W All zeros 8.5.3.3.17/-135            */         
#define FMBM_REFQID					0x064 				/*				Rx Error Frame Queue ID R/W All zeros 8.5.3.3.18/-136      */         
#define FMBM_RFSDM				0x068 					/*		Rx Frame Status Discard Mask R/W All zeros 8.5.3.3.19/-136     */         
#define FMBM_RFSEM				0x06C 					/*			Rx Frame Status Error Mask R/W All zeros 8.5.3.3.20/-137     */         
#define FMBM_RFENE					0x070 				/*			Rx Frame Enqueue Next Engine R/W 0x00D4_0000 8.5.3.3.21/-13  */      


#define FMBM_PP(n)                   0x104+(4*(n-1))
#define FMBM_PFS(n)                  0x204+(4*(n-1))
#define FMBM_SPLIODN(n)               0x304+(4*(n-1))


#define FMBM_RMPD                   0x180
#define FMBM_RSTC                   0x200
#define FMBM_RFRC                   0x204
#define FMBM_RBFC                   0x208
#define FMBM_RLFC                   0x20c
#define FMBM_RFFC                   0x210
#define FMBM_RFDC                   0x214
#define FMBM_RFLDEC                 0x218
#define FMBM_RODC                   0x21c
#define FMBM_RBDC                   0x220
#define FMBM_RPC                    0x280
#define FMBM_RPCP                   0x284
#define FMBM_RCCN                   0x288
#define FMBM_RTUC                   0x28c
#define FMBM_RRQUC                  0x290
#define FMBM_RDUC                   0x294
#define FMBM_RFUC                   0x298
#define FMBM_RPAC                   0x29c

#define FMBM_TRLMTS         0x002C/* Tx Rate Limiter Scale Register (FMBM_TRLMTS) */
#define FMBM_TRLMT                          0x0030                               /* Tx Rate Limiter Register (FMBM_TRLMT) */

#define  FMPR_PxCAC               0x3F8     /*—Port x Configuration Access Control Mixed 0x0000_0000 /8-242*/
#define  FMPR_PxCTPID             0x3FC /*—Port x Configured TPID R/W 0x9100_9100 /8-243*/
#define FMPR_SXPAW0               0x800
#define FMPR_SXPAW1               0x804
#define FMPR_SXPAW2               0x808
#define FMPR_SXPAW3               0x80c
#define FMPR_SXPAW4               0x810
#define FMPR_SXPAW5               0x814
#define FMPR_SXPAW6               0x818
#define FMPR_SXPAW7               0x81c
#define FMPR_SXPAW8               0x820
#define FMPR_SXPAW9               0x824
#define FMPR_SXPAW10               0x828
#define FMPR_SXPAW11              0x82c
#define FMPR_SXPAW12               0x830
#define FMPR_SXPAW13               0x834
#define FMPR_SXPAW14               0x838
#define FMPR_SXPAW15               0x83c
#define FMPR_SXPAW16               0x840
#define FMPR_SXPAW17               0x844
#define FMPR_SXPAW18               0x848

#define FMPR_PARSE_MEM 0x0
#define FMPR_PEVR      0x860       /*    —Parser Event Register w1c 0x0000_0000 /8-246           */   
#define FMPR_PEVER     0x864       /*     —Parser Event Enable Register R/W 0x0000_0000 /8-247   */
#define FMPR_PERR      0x86C       /*   —Parser Error Register w1c 0x0000_0000 /8-248            */          
#define FMPR_PERER     0x870       /*        —Parser Error Enable Register R/W 0x0000_0000 /8-248*/         
#define FMPR_PPSC      0x8a0
#define FMPR_PDS       0x8a8
#define FMPR_L2RRS     0x8ac
#define FMPR_L3RRS     0x8b0
#define FMPR_L4RRS     0x8b4
#define FMPR_SRRS      0x8b8
#define FMPR_L2RRES    0x8bc
#define FMPR_L3RRES    0x8c0
#define FMPR_L4RRES    0x8c4
#define FMPR_SRRES     0x8c8
#define FMPR_SPCS      0x8cc
#define FMPR_SPSCS     0x8d0
#define FMPR_HXSCS     0x8d4
#define FMPR_MRCS      0x8d8
#define FMPR_MWCS      0x8dc
#define FMPR_MRSCS     0x8e0
#define FMPR_MWSCS     0x8e4



#define ETH_DESEC1     0x4E0000
#define ETH_DESEC2     0x4E2000
#define ETH_DESEC3     0x4E4000
#define ETH_DESEC4     0x4E6000
#define ETH_DESEC5     0x4E8000
#define ETH_MAC_RBYT   0x21c
#define ETH_MAC_RPKT   0x220
#define ETH_MAC_TBYT   0x260
#define ETH_MAC_TPKT   0x264

#define FMAN_ETH1_TX_PORTID 0x28
#define FMAN_ETH1_RX_PORTID 0x8
#define FMAN_ETH2_TX_PORTID 0x29
#define FMAN_ETH2_RX_PORTID 0x9
#define FMAN_ETH3_TX_PORTID 0x2a
#define FMAN_ETH3_RX_PORTID 0xa
#define FMAN_ETH4_TX_PORTID 0x2b
#define FMAN_ETH4_RX_PORTID 0xb
#define FMAN_ETH5_TX_PORTID 0x2c
#define FMAN_ETH5_RX_PORTID 0xc


#define FMQMI_GC          0x400
#define FMQMI_EIE         0x408
#define FMQMI_EIEN        0x40c
#define FMQMI_EIF         0x410
#define FMQMI_IE          0x414
#define FMQMI_IEN         0x418
#define FMQMI_IF          0x41c
#define FMQM_GS           0x420
#define FMQM_ETFC         0x428
#define FMQM_DTFC         0x42c
#define FMQM_DC0          0x430
#define FMQM_DC1          0x434
#define FMQM_DC2          0x438
#define FMQM_DC3          0x43c
#define FMQM_TAPC         0x46c
#define FMQM_DMCVC        0x470
#define FMQM_DIFDCC       0x474
#define FMQM_DA1VC        0x478
#define FMQM_DTRC         0x480
#define FMQM_EFDDD        0x484
#define FMQM_DTCA1        0x490
#define FMQM_DTVA1        0x494
#define FMQM_DTMA1        0x498
#define FMQM_DTCA         0x49c

#define FMQM_DTCA2        0x4a0
#define FMQM_DTVA2        0x4a4
#define FMQM_DTMA2        0x4a8

#define FMQM_PNC          0x400
#define FMQM_PNS          0x404
#define FMQM_PNTS         0x408
#define FMQM_PNEN         0x41c
#define FMQM_PNETFC       0x420
#define FMQM_PNDN         0x42c
#define FMQM_PNDC         0x430
#define FMQM_PNDTFC       0x434
#define FMQM_PNDFNOC      0x438
#define FMQM_PNDCC        0x43c




#endif /*  __HEADER_FM_REGS_H */
/******************************* 头文件结束 ********************************/

