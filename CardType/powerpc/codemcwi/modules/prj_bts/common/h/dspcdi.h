
#ifndef __BTS_DSPCDI_H__
#define __BTS_DSPCDI_H__

typedef struct{
	short real;
	short imag;
}complex16;

//FEP CDI message:
typedef struct{
 int FepId;
 int fpgaVer;			//FEP FPGA version
 int btsSeqId;
 int TDDtimeslot;
 int Dnlktimeslot;
 int preambleScale;
 int antennaMask;
 int SCGMask;
 complex16 w0[8];		//w0 form L2
 int auxmsgCnt[4];		//0：分配的链表数目 1：接受DMA计数 2：发送DMA计数 3：释放的链表数目
 int auxChkErr;			//接收的AUX消息中Chksum错误计数
 int cPreUplk;			//preuplink count
 int cUplk;				//uplink count
 int cPreDnlk;			//predownlink count
 int cDnlk;				//downlink count
 int cDefault;			//TDD 中断错误计数
 int dnlkCount[8];      //total downlink data from mcp count
 int dnlkISymbErrCount[8];		//downlink data symb erroe count
 int dnlkStatusErrCount[8];		//downlink from mcp status register error count 
 int dnlkErrCount[8];   //downlink data from mcp checksum error count
 int cErrChkSymb[34];           //downlink data from mcp checksum error at which symb
 int bMpTest;                   //mp test flag
 int bRevTest;                  //reverse test flag
 int RevType;                   //reverse test type
}FEPCDIBufType;

#ifdef WBBU_CODE
typedef struct
{
	int FepId;	//32	0：fep0，1：fep1
	int btsSeqId	;//32	基站序列号
	int TDDtimeslot;//	32	总时隙数
	int Dnlktimeslot	;//32	下行时隙数
	int preambleScale;	//32	同步头Scale值
	int antennaMask;	//32	天线Mask
	int SCGMask ;	//32	SCGMask
	int W0[8]	;//32*8	W0
	int auxmagCnt[2]	;//32*2	FEP与AUX通信计数，0：收到校准数据次数，1：收到Config消息次数
	int auxChkErr;	//32	FEP收到AUX数据校验和错误次数
	int cUplk	;//32	收到FPGA的接收中断次数
	int cDnlk	;//32	收到FPGA的发射中断次数
	int dnlkCount[8];	//32*8	收到8个MCP数据的次数，分别统计各个MCP的
	int dnlkISymbErrCount[8];//	32*8	收到8个MCP数据时隙号不对的次数，分别统计各个MCP，此结果在MCP没有运行的情况下没有意义
	int dnlkErrCount[8]	;//32*8	收到8个MCP数据校验和错误的次数，分别统计各个MCP的
	int cErrChkSymb[34];	//32*34	收到8个MCP每个Symbol校验和错误的次数，8个MCP累加结果，只上报34个Symbol结果
	int bMpTest	;//32	低16bit：测试变量，目前未用，高16bit：400M脉冲干扰抵消开关（1为开）
	int bRevTest;//32	低16bit：测试变量，目前未用，高16bit：最大脉冲值
	int RevType;	//32	低16bit：测试变量，目前未用，高16bit：最小脉冲值
	int FlagTestFpgaUp;	//32	FEP检测FPGA上行数据开关，1为开
	int FlagTestFpgaDn	;//32	FEP下行数据为测试数据开关，1为开
	int UpDataErrCnt[2]	;//32*2	FEP检测上行测试数据计数，0：正确帧数，1：错误帧数


}FEPStatusBuffer;
#endif
//AUX CDI message:

typedef struct{
 int fpgaVer;			//L1 FPGA version
 int btsSeqId;
 int TDDtimeslot;
 int Dnlktimeslot;
 int syncSRC;
 int Gps_offset;  
 int antennaMask;
 int rfPoweron;
 complex16 w0[8];
 int RfRxGain[8];		//RF rx gain
 int RfTxGain[8];		//RF tx gain
 int SynRxGain;
 int SynTxGain;
 int L2msgCnt[5];		//0:接收L2FPGA DMA满中断数 1：接收DMA挂上链表数目 2：DMA checksum Error count 3: Trans DMA count 4:receive L2 FPGA empty interrupt count
 int L2TimeCnt[3];		//0:receive L2 FPGA timer interrupt count 1:hang to schedulelist count 2:receive downlink weight checksum error count
 int fepChkErr[4];		//0:receive fep DMA checksum 1:receive fep DMA length 2:FEP0 DMAchecksum error 3:FEP1 DMA checksum error
 int spiChkErr[11];		//0-7:RF channel 0-7 checksum error count 8:calibration SYN chksum Error count 9:read SYN status chksum error count 10: read SYN temperature chksum error count
 int calibErrflag;		//calibration error status
 int emifaNotEmpty;		//emifa not empty at uplink to clear schedulelist count
 int emifbNotEmpty;		//emifb not empty at uplink to clear schedulelist count
}AUXCDIBufType;


#ifdef WBBU_CODE
typedef struct
{
	int btsSeqId;	//32	基站序列号
	int TDDtimeslot;	//32	总TDD时隙数
	int Dnlktimeslot;	//32	下行时隙数
	int syncSRC;	//32	高层下发的参数
	int antennaMask;//	32	天线Mask
	int w0[8]	;//32*8	W0
	int RfRxGain[8];//	32*8	8块TR板接收增益值
	int RfTxGain[8];	//32*8	8块TR板发射增益值
	int SynRxGain;//	32	Syn接收增益值
	int SynTxGain	;//32	Syn发射增益值
	int L2msgCnt[3]	;//32*3	AUX与Core9通信计数，0：收到组合消息包次数，AUX与1：校验和正确次数，2：校验和错误次数
	int fepChkErr	[2];//32*2	AUX与FEP通信校验和错误计数，0：fep0，1：fep1
	int calibErrflag	;//32	通道校准结果
	int SynParaFlag;	//32	AUX是否收到L3下发的Syn校表
	int RfCalTxGain[10];	//32*10	校准过程中的所有TR板发射增益值（5*8*8bit，5次调整增益）
	int RfCalRxGain[12];	//32*12	校准过程中的所有TR板接收增益值（6*8*8bit，6次调整增益）
	int SynCalRxGain[2];	//32*2	校准过程中的所有Syn接收增益值（4*16bit，4次调整增益）
	int SynCalTxGain[13];	//32*13	校准过程中的所有Syn发射增益值（5*10*16bit，5次调整增益），rsv16bit
	int GpioRevCount[3];	//32*3	AUX收到L1的Gpio中断次数，0：DSP2，1：DSP3，2：Dsp4
	int GpioDataHeadMissCount[3];	//32*3	AUX与L1通信丢失数据头次数，同上
	int GpioDataHeadOkCount[3];	//32*3	AUX与L1通信正确收到数据头次数，同上
	int link_state_counter[3];	//32*3	AUX与L1之间Link状态错误次数，同上

}AUXStatusBuffer;
#endif
//MCP CDI message

typedef struct
{
	int test_uplink_report_tsn_err;
	int test_ppc_length_zeros;
	int test_ppc_status_empty;
	int test_ppc2mcp_checksum_err;          
	int test_ppc_status_invalid;
	int test_ppc2mcp_int_counter_err;
	int test_ppc2mcp_tsn_err;
	int test_downlink_proc_err_times;
	int test_TDD_EDMA_not_over_counter;
	int test_PPC2MCPint_EDMA_not_over_counter;
	int test_ppc_length_too_long;
	int test_mcp2ppc_length_too_long;	
	int test_mcp2ppc_EDMA_not_over_counter;
	int test_mcp2ppc_status_not_empty_err;	
	int test_fep0_checksum_err;
	int test_fep1_checksum_err;	
	int test_fep01_status_len_err;
	int test_fep0_status_len_err;
	int test_fep1_status_len_err;
	int test_symb_idx_err;
	int test_mcpId_err_counter;
	int test_TDD;
	int test_uplink_deal_times;
	int test_ppc2mcp_int;
	int test_edma_over;        	
	int test_deal_with_ppc_data;	
	int test_before_start_mcp2ppc_dma_counter;
	int test_start_ppc2mcp_dma_counter;
	int test_downlink_proc_times;
	int test_fep0_read;
	int test_fep1_read;
	int test_fep_read;
	int test_start_mcp2ppc_dma_counter;
	int test_shutdown_nulling;
	int mcpId;
	int sch_num;
	int schIdx_begin;
	int schIdx_end;
	int Downlink_DC_sch_posi;
	int Downlink_DC_sca_posi;
	int uplink_DC_sch_posi;
	int uplink_DC_sca_posi;
	int uplink_DC_sch_posi2;
	int uplink_DC_sca_posi2;
	int two_SCG;
	int sch_in_first_SCG;
	int BTS_sensitivity;
	int BTS_power;			
	int mcp2fep_dma_size;
}MCPCDIBufType;	
#ifdef WBBU_CODE
typedef struct
{
	int Frame_Counter;
	
	int Recv_AUX_GPIO_counter;
	int Recv_AUX_AIF_Get_counter;
	int Recv_AUX_AIF_Lost_counter;
	int Recv_AUX_EDMA_counter;
	int Recv_AUX_AIF_Link_Error_counter;

	int Recv_DSP2_GPIO_counter;
	int Recv_DSP2_AIF_Get_counter;
	int Recv_DSP2_AIF_Lost_counter;
	int Recv_DSP2_EDMA_counter;
	int Recv_DSP2_AIF_Link_Error_counter;

	int Recv_DSP3_GPIO_counter;
	int Recv_DSP3_AIF_Get_counter;
	int Recv_DSP3_AIF_Lost_counter;
	int Recv_DSP3_EDMA_counter;
	int Recv_DSP3_AIF_Link_Error_counter;
	
	int Recv_DSP4_EDMA_counter;
	
	int Recv_MAC_GPIO_counter;
	int Recv_MAC_AIF_Get_counter;
	int Recv_MAC_AIF_Lost_counter;
	int Recv_MAC_EDMA_counter;
	int Recv_MAC_AIF_Link_Error_counter;

	int Recv_MCP0_Error_Counter;
	int Recv_MCP1_Error_Counter;	
	int Recv_MCP2_Error_Counter;	
	int Recv_MCP3_Error_Counter;	
	int Recv_MCP4_Error_Counter;	
	int Recv_MCP5_Error_Counter;
	int Recv_MCP6_Error_Counter;	
	int Recv_MCP7_Error_Counter;	
	int Recv_MAC_Error_Counter;

	int Send_MCP_Counter;
	int Send_AUX_Counter;
	int Send_MAC_Counter;

	int MCP_Proc_Counter;
	int MAC_Proc_Counter;
	int AUX_Proc_Counter;

	int AIF_Reconfig_Counter;
} Core9TestCounterControl;
#endif

#endif
