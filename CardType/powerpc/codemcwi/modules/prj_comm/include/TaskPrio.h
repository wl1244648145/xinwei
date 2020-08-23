#ifndef __TASK_PRI_DEF
//Task Priority definitions
//Layer 3
#define M_TP_CSI_WDT               0
#define M_TP_IDLE_PERF             29
#ifndef WBBU_CODE
#define M_TP_L2IF                  48//35 //48 liuweidong for l2if deadlock
#else
#define M_TP_L2IF                  90
#endif
#define M_TP_L3EB                 100
#ifdef M_TGT_WANIF
#define M_TP_L3WANIF          100
#endif
#define M_TP_L3EMSAGENTTX         105
//#define M_TP_L3TCR                105
#define M_TP_L3TUNNEL             106
//#define M_TP_L3TDR                107
#define M_TP_L3SNOOP              108
#define M_TP_L3ARP                109
#define M_TP_L3DM                 115
#define M_TP_L3VCR                119
#define M_TP_L3VOICE              120
#define M_TP_L3VDR                121
#define M_TP_L3BM                 125
#define M_TP_L3SYS                130
#define M_TP_L3CM                 132
#define M_TP_L3UM                 133
#define M_TP_L3SM                 134
#define M_TP_L3EMSAGENTRX         136
#define M_TP_L3FTPCLIENT          138
#define M_TP_L3DIAGM              140
#define M_TP_L3GM                 160
#define M_TP_L3FM                 165
#define M_TP_L3PM                 170
#define M_TP_L3CLEANUP            190
#define M_TP_DIAG_SHELL           200
#define M_TP_L2_SHELL             210
#define M_TP_L2_SHELL_TX          220
#define M_TP_L2_SHELL_RX          230
#define M_TP_FTPC                 240
#define M_TP_L3LOG                250
#define M_TP_CSI_STACK_CHECK      251
#define M_TP_CSI_IDLE             255
#ifdef WBBU_CODE
#define  M_TP_L3RM                  124
#endif


//Layer 2
#define M_TP_L2BOOT	              65
#define M_TP_L3IF                 87
#define M_TP_L2L3AGENT
#define M_TP_L2DAC                100
#define M_TP_L2VAC
#define M_TP_L2DIAG
#define M_TP_L1TDDIF              50
#define M_TP_AUXCTRLIF            60

#define M_TP_SIO_TX               200
#define M_TP_SIO_RX               210

#define M_TP_PCIF              11
#define M_TP_L1L2IF            6
#define M_TP_PCMCIA_IF         7
#define M_TP_CPEL2TX           8
#define M_TP_CPEL2MAIN         9

#ifndef BF_NU_L2
#define M_TP_ETHIF             11
#else
#define M_TP_ETHIF             11  //7
#endif
#define M_TP_USBIF             11

#define M_TP_UTV               15
#define M_TP_UTDM              16
#define M_TP_CPECM             17
#define M_TP_CPESM             30	//zmy_test 18
#define M_TP_CPEDIAG           19


//common tasks
#ifndef __NUCLEUS__
#define M_TP_TT                220
#else
#define M_TP_TT                20		//zmy_test 30
#endif

#endif  //#ifndef __TASK_PRI_DEF
