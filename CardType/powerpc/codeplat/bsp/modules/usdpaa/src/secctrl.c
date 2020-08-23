/******************************************************************************

 **************************************************************************/

/* Include the external interfaces of other modules */

/* Include the internal definitions of this module */
#include "../inc/secctrl.h"
#include <stdio.h>
#define SEC_ADDR_BASE                         (unsigned long)0x300000
#define SEC_JOBRING0_OFFSET                   (unsigned long)0x1000
#define SEC_QI_OFFSEC                         (unsigned long)0x7000

extern unsigned char  *g_u8ccsbar;

void *t_Sec = 0;
t_SecGenMemMap*   p_SecMemMap = 0; /* Memory-mapped registers */
t_SecJqMemMap*		p_SecJq0MemMap = 0;
t_SecQiMemMap*		p_QiMemMap = 0;

void DumpSecReg(void)
{
    printf("\n ************************* SEC HANDLE *********************************\n");
	printf("\n  t_Sec = 0x%p \n", t_Sec);
	printf("\n  p_SecMemMap = 0x%p \n", p_SecMemMap);
	printf("\n  p_SecJq0MemMap = 0x%p \n", p_SecJq0MemMap);
    printf("\n  p_QiMemMap = 0x%p \n", p_QiMemMap);
    
    printf("\n ************************* SEC GENERAL REGISTER *********************************\n");
    printf("  mcfgr = 0x%x \n",       BSP_GET_BIT32(p_SecMemMap->gen.mcfgr));
	printf("  jq0liodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.jq0liodnr));
	printf("  jq1liodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.jq1liodnr));
	printf("  jq2liodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.jq2liodnr));
	printf("  jq3liodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.jq3liodnr));
    printf("  rticaliodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.rticaliodnr));
	printf("  rticbliodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.rticbliodnr));
	printf("  rticcliodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.rticcliodnr));
	printf("  rticdliodnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->gen.rticdliodnr));
	printf("  decoreqr = 0x%x \n",    BSP_GET_BIT32(p_SecMemMap->gen.decoreqr));
	printf("  deco0liodnr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->gen.deco0liodnr));
	printf("  deco1liodnr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->gen.deco1liodnr));
	printf("  deco2liodnr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->gen.deco2liodnr));
	printf("  deco3liodnr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->gen.deco3liodnr));
	printf("  deco4liodnr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->gen.deco4liodnr));
	printf("  decoavlr = 0x%x \n",    BSP_GET_BIT32(p_SecMemMap->gen.decoavlr));
	printf("  decorstr = 0x%x \n",    BSP_GET_BIT32(p_SecMemMap->gen.decorstr));
	printf("  jdkekr[0] = 0x%x \n",   BSP_GET_BIT32(p_SecMemMap->gen.jdkekr[0]));
	printf("  tdkekr[0] = 0x%x \n",   BSP_GET_BIT32(p_SecMemMap->gen.tdkekr[0]));
	printf("  tdskr[0] = 0x%x \n",    BSP_GET_BIT32(p_SecMemMap->gen.tdskr[0]));
	printf("  sknr = 0x%llx \n",      BSP_GET_BIT64(p_SecMemMap->gen.sknr));
	printf("\n ************************* SEC STATUS REGISTER *********************************\n");
	printf("  req_deq = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.req_deq));
	printf("  ob_enc_req = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ob_enc_req));
	printf("  ib_dec_req = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ib_dec_req));
	printf("  ob_encrypt = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ob_encrypt));
	printf("  ob_protect = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ob_protect));
	printf("  ib_decrypt = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ib_decrypt));
	printf("  ib_validated = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ib_validated));
	printf("  crnr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.crnr));
	printf("  ctpr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.ctpr));
	printf("  far = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.far));
	printf("  falr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->stat.falr));
	printf("  fadr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->stat.fadr));
	printf("  sstar = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->stat.sstar));
	printf("  rvidr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->stat.rvidr));
	printf("  ccbvidr = 0x%x \n", BSP_GET_BIT32(p_SecMemMap->stat.ccbvidr));
	printf("  chavidr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.chavidr));
	printf("  chanumr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.chanumr));
	printf("  secvidr = 0x%llx \n", BSP_GET_BIT64(p_SecMemMap->stat.secvidr));

	printf("\n ************************* SEC JQ0 REGISTER *********************************\n");
    printf("  irbar = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->jq.irbar));
	printf("  irsr = 0x%x \n", BSP_GET_BIT32(p_SecJq0MemMap->jq.irsr));
	printf("  irsar = 0x%x \n", BSP_GET_BIT32(p_SecJq0MemMap->jq.irsar));
	printf("  irjar = 0x%x \n", BSP_GET_BIT32(p_SecJq0MemMap->jq.irjar));
	printf("  orbar = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->jq.orbar));
	printf("  orsr = 0x%x \n",   BSP_GET_BIT32(p_SecJq0MemMap->jq.orsr));
	printf("  cfgr = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->jq.cfgr));

	printf("\n ************************* SEC JQ0 STATUS REGISTER *********************************\n");
	printf("  req_deq = 0x%llx \n",    BSP_GET_BIT64(p_SecJq0MemMap->stat.req_deq));
	printf("  ob_enc_req = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->stat.ob_enc_req));
	printf("  ib_dec_req = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->stat.ib_dec_req));
	printf("  ob_encrypt = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->stat.ob_encrypt));
	printf("  ob_protect = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->stat.ob_protect));
	printf("  ib_decrypt = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->stat.ib_decrypt));
	printf("  ib_validated = 0x%llx \n", BSP_GET_BIT64(p_SecJq0MemMap->stat.ib_validated));

	printf("\n ************************* SEC QI REGISTER *********************************\n");
    printf("  ctlr = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->qi.ctlr));
    printf("  star = 0x%x \n",   BSP_GET_BIT32(p_QiMemMap->qi.star));
	printf("  dqcr = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->qi.dqcr));
	printf("  eqcr = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->qi.eqcr));
	printf("  lcr = 0x%llx \n",  BSP_GET_BIT64(p_QiMemMap->qi.lcr));

	
	printf("\n ************************* SEC QI STATUS REGISTER *********************************\n");
	printf("  req_deq = 0x%llx \n",    BSP_GET_BIT64(p_QiMemMap->stat.req_deq));
	printf("  ob_enc_req = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->stat.ob_enc_req));
	printf("  ib_dec_req = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->stat.ib_dec_req));
	printf("  ob_encrypt = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->stat.ob_encrypt));
	printf("  ob_protect = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->stat.ob_protect));
	printf("  ib_decrypt = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->stat.ib_decrypt));
	printf("  ib_validated = 0x%llx \n", BSP_GET_BIT64(p_QiMemMap->stat.ib_validated));
	
}


unsigned long bsp_sec_reg_init(void)
{
       unsigned long long qi_ctlr = 0;
	t_Sec= (void *)((unsigned long)g_u8ccsbar + SEC_ADDR_BASE);
	p_SecMemMap = (t_SecGenMemMap*)(t_Sec);
	p_SecJq0MemMap = (t_SecJqMemMap*)(t_Sec + SEC_JOBRING0_OFFSET);
	p_QiMemMap = (t_SecQiMemMap*)(t_Sec + SEC_QI_OFFSEC);
	/* enable QI */
	qi_ctlr = BSP_GET_BIT64(p_QiMemMap->qi.ctlr);
	qi_ctlr |= CTLR_DQEN;
	BSP_WRITE_BIT64(p_QiMemMap->qi.ctlr, qi_ctlr);

	/* clear counter */
	BSP_WRITE_BIT64(p_SecMemMap->stat.req_deq, 0);
	BSP_WRITE_BIT64(p_SecMemMap->stat.ob_enc_req, 0);
	BSP_WRITE_BIT64(p_SecMemMap->stat.ib_dec_req, 0);
	BSP_WRITE_BIT64(p_SecMemMap->stat.ob_encrypt, 0);
	BSP_WRITE_BIT64(p_SecMemMap->stat.ob_protect, 0);
	BSP_WRITE_BIT64(p_SecMemMap->stat.ib_decrypt, 0);
	BSP_WRITE_BIT64(p_SecMemMap->stat.ib_validated, 0);

	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.req_deq, 0);
	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.ob_enc_req, 0);
	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.ib_dec_req, 0);
	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.ob_encrypt, 0);
	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.ob_protect, 0);
	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.ib_decrypt, 0);
	BSP_WRITE_BIT64(p_SecJq0MemMap->stat.ib_validated, 0);

	BSP_WRITE_BIT64(p_QiMemMap->stat.req_deq, 0);
	BSP_WRITE_BIT64(p_QiMemMap->stat.ob_enc_req, 0);
	BSP_WRITE_BIT64(p_QiMemMap->stat.ib_dec_req, 0);
	BSP_WRITE_BIT64(p_QiMemMap->stat.ob_encrypt, 0);
	BSP_WRITE_BIT64(p_QiMemMap->stat.ob_protect, 0);
	BSP_WRITE_BIT64(p_QiMemMap->stat.ib_decrypt, 0);
	BSP_WRITE_BIT64(p_QiMemMap->stat.ib_validated, 0);
	
	DumpSecReg();
	return 0;
}
