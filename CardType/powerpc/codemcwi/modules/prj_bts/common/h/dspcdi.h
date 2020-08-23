
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
 int auxmsgCnt[4];		//0�������������Ŀ 1������DMA���� 2������DMA���� 3���ͷŵ�������Ŀ
 int auxChkErr;			//���յ�AUX��Ϣ��Chksum�������
 int cPreUplk;			//preuplink count
 int cUplk;				//uplink count
 int cPreDnlk;			//predownlink count
 int cDnlk;				//downlink count
 int cDefault;			//TDD �жϴ������
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
	int FepId;	//32	0��fep0��1��fep1
	int btsSeqId	;//32	��վ���к�
	int TDDtimeslot;//	32	��ʱ϶��
	int Dnlktimeslot	;//32	����ʱ϶��
	int preambleScale;	//32	ͬ��ͷScaleֵ
	int antennaMask;	//32	����Mask
	int SCGMask ;	//32	SCGMask
	int W0[8]	;//32*8	W0
	int auxmagCnt[2]	;//32*2	FEP��AUXͨ�ż�����0���յ�У׼���ݴ�����1���յ�Config��Ϣ����
	int auxChkErr;	//32	FEP�յ�AUX����У��ʹ������
	int cUplk	;//32	�յ�FPGA�Ľ����жϴ���
	int cDnlk	;//32	�յ�FPGA�ķ����жϴ���
	int dnlkCount[8];	//32*8	�յ�8��MCP���ݵĴ������ֱ�ͳ�Ƹ���MCP��
	int dnlkISymbErrCount[8];//	32*8	�յ�8��MCP����ʱ϶�Ų��ԵĴ������ֱ�ͳ�Ƹ���MCP���˽����MCPû�����е������û������
	int dnlkErrCount[8]	;//32*8	�յ�8��MCP����У��ʹ���Ĵ������ֱ�ͳ�Ƹ���MCP��
	int cErrChkSymb[34];	//32*34	�յ�8��MCPÿ��SymbolУ��ʹ���Ĵ�����8��MCP�ۼӽ����ֻ�ϱ�34��Symbol���
	int bMpTest	;//32	��16bit�����Ա�����Ŀǰδ�ã���16bit��400M������ŵ������أ�1Ϊ����
	int bRevTest;//32	��16bit�����Ա�����Ŀǰδ�ã���16bit���������ֵ
	int RevType;	//32	��16bit�����Ա�����Ŀǰδ�ã���16bit����С����ֵ
	int FlagTestFpgaUp;	//32	FEP���FPGA�������ݿ��أ�1Ϊ��
	int FlagTestFpgaDn	;//32	FEP��������Ϊ�������ݿ��أ�1Ϊ��
	int UpDataErrCnt[2]	;//32*2	FEP������в������ݼ�����0����ȷ֡����1������֡��


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
 int L2msgCnt[5];		//0:����L2FPGA DMA���ж��� 1������DMA����������Ŀ 2��DMA checksum Error count 3: Trans DMA count 4:receive L2 FPGA empty interrupt count
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
	int btsSeqId;	//32	��վ���к�
	int TDDtimeslot;	//32	��TDDʱ϶��
	int Dnlktimeslot;	//32	����ʱ϶��
	int syncSRC;	//32	�߲��·��Ĳ���
	int antennaMask;//	32	����Mask
	int w0[8]	;//32*8	W0
	int RfRxGain[8];//	32*8	8��TR���������ֵ
	int RfTxGain[8];	//32*8	8��TR�巢������ֵ
	int SynRxGain;//	32	Syn��������ֵ
	int SynTxGain	;//32	Syn��������ֵ
	int L2msgCnt[3]	;//32*3	AUX��Core9ͨ�ż�����0���յ������Ϣ��������AUX��1��У�����ȷ������2��У��ʹ������
	int fepChkErr	[2];//32*2	AUX��FEPͨ��У��ʹ��������0��fep0��1��fep1
	int calibErrflag	;//32	ͨ��У׼���
	int SynParaFlag;	//32	AUX�Ƿ��յ�L3�·���SynУ��
	int RfCalTxGain[10];	//32*10	У׼�����е�����TR�巢������ֵ��5*8*8bit��5�ε������棩
	int RfCalRxGain[12];	//32*12	У׼�����е�����TR���������ֵ��6*8*8bit��6�ε������棩
	int SynCalRxGain[2];	//32*2	У׼�����е�����Syn��������ֵ��4*16bit��4�ε������棩
	int SynCalTxGain[13];	//32*13	У׼�����е�����Syn��������ֵ��5*10*16bit��5�ε������棩��rsv16bit
	int GpioRevCount[3];	//32*3	AUX�յ�L1��Gpio�жϴ�����0��DSP2��1��DSP3��2��Dsp4
	int GpioDataHeadMissCount[3];	//32*3	AUX��L1ͨ�Ŷ�ʧ����ͷ������ͬ��
	int GpioDataHeadOkCount[3];	//32*3	AUX��L1ͨ����ȷ�յ�����ͷ������ͬ��
	int link_state_counter[3];	//32*3	AUX��L1֮��Link״̬���������ͬ��

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
