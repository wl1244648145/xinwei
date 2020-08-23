/******************************************************************************

 **************************************************************************/

/* Include the external interfaces of other modules */
#include "../inc/bsp.h"
#include "../inc/compat.h"
#include "../inc/TBuf.h"

#include "../inc/fsl_qman.h"
#include "../inc/fsl_shmem.h"


/* Include the internal definitions of this module */
#include "../inc/dcl.h"
#include "../inc/secapp.h"
#include "../inc/sec_api.h"
#include "../inc/sec_regs.h"
#include "../inc/reg.h"

#ifdef BSP_DEBUG
#define XOS_GETUB malloc
#else
#include "xos_external.h"
#endif


#define AES_BLOCK_SIZE (16 * 8) /* number of bits */

#define SEC_ADDR_BASE			(unsigned long)0x300000
#define SEC_JOBRING0_OFFSET		(unsigned long)0x1000
#define JRINT_ERR_HALT_MASK		0xc
#define JRINT_ERR_HALT_INPROGRESS	0x4
#define JRINT_ERR_HALT_COMPLETE		0x8

#define JRCR_RESET		0x01
#define JOBR_DEPTH		16
#define MAX_PACKAGE_SIZE	(8 * 1024)

#define MAX_KEY_SIZE		16 /* 128bits */

#define OUTPUT_INC(x) (x + (sizeof(phy_addr_t) + sizeof(uint32_t)) / sizeof(uint32_t))
#define MAX_BUFFER_SIZE MAX_PACKAGE_SIZE

enum {
	EEA0,	/* Null ciphering algorithm */
	EEA1,	/* SNOW 3G based algorithm */
	EEA2,	/* AES based algorithm */
};

enum {
	EIA0,
	EIA1,
	EIA2,
};

enum {
	ENCRYPTION,
	INTEGRITY,
};
/*
 * because ddr ram size < 4G, so phy_addr_t use 32bit is safe
 * and usdpaa use 32bit physical addr
 * If ddr memory size > 4G it will cause bug, and must porting
 * usdpaa from 32bit to 64bit physical address.
 */
typedef uint32_t phy_addr_t;

struct addr_info {
	phy_addr_t pa;
	void *va;
};


struct sec_ram_map{
	uint32_t desc[JOBR_DEPTH][MAX_CAAM_DESCSIZE];
	uint32_t i_ring_entry[JOBR_DEPTH][1];
	uint32_t o_ring_entry[JOBR_DEPTH][2];
	uint8_t src[JOBR_DEPTH][MAX_BUFFER_SIZE];
	uint8_t dst[JOBR_DEPTH][MAX_BUFFER_SIZE];
	uint8_t key[JOBR_DEPTH][MAX_BUFFER_SIZE];
	uint8_t iv[JOBR_DEPTH][MAX_BUFFER_SIZE];
};

struct sec_jr {
	uint32_t head;
	uint32_t tail;
	spinlock_t lock;
};

struct sec_info {
	t_SecJqRegs *regs;
	t_SecGenRegs *gen;
	struct sec_jr i_ring;	
	struct sec_jr o_ring;	
	struct sec_ram_map *virt_addr;
	phy_addr_t phy_addr;
	int buf_index;
};

struct crypto_info {
	uint8_t *src;
	uint8_t *dst;
	uint8_t *key;
	uint32_t *iv;
	uint32_t len;
	uint32_t keysize;
	uint32_t crypto_type;
	uint32_t alg_type;
	enum algdir dir;
	uint8_t index;
};


static struct sec_info sec;

extern int cnstr_jobdesc_snow_f8(uint32_t *descbuf, uint16_t *bufsize,
		uint8_t *key, uint32_t keylen,
		enum algdir dir,  uint32_t *ctx,
		uint8_t *in, uint8_t *out, uint32_t size);
extern int cnstr_jobdesc_aes_ctr(uint32_t *descbuf, uint16_t *bufsize,
		uint8_t *key, uint32_t keylen,
		enum algdir dir,  uint32_t *ctx,
		uint8_t *in, uint8_t *out, uint16_t size);
extern int cnstr_jobdesc_snow_f9(uint32_t *descbuf, uint16_t *bufsize,
		uint8_t *key, uint32_t keylen,
		enum algdir dir, uint32_t *ctx,
		uint8_t *in, uint16_t size, uint8_t *mac);
extern int cnstr_jobdesc_aes_cmac_multi_fd(uint32_t *descbuf, uint16_t *bufsize,
		uint8_t *key, uint32_t keylen,
		enum algdir dir,
		uint8_t *in, uint16_t size, uint8_t *mac);
extern int cnstr_jobdesc_aes_cmac(uint32_t *descbuf, uint16_t *bufsize,
		uint8_t *key, uint32_t keylen,
		enum algdir dir,
		uint8_t *in, uint16_t size, uint8_t *mac);

extern unsigned long bsp_sec_reg_init(void);
extern void dpadelay(unsigned int count);
extern void DumpSecReg(void);
extern ULONG bsp_find_phyaddr_from_tbl(unsigned char *pname,unsigned int dwindex);
extern void *bsp_shm_p2v(unsigned long dwphyaddr);

extern unsigned char  *g_u8ccsbar;
extern t_SecGenMemMap *p_SecMemMap;
extern t_SecJqMemMap *p_SecJq0MemMap; 

static inline phy_addr_t __pa(uint8_t *va)
{
	if (va >= (uint8_t *)sec.virt_addr && va <= (0x100000 + (uint8_t *)sec.virt_addr))
		return (phy_addr_t)(va - (uint8_t *)sec.virt_addr) + sec.phy_addr;
	else
		return ub_va_to_pa(va); 
}

#define JOB_INC(x) (x = (x + 1) & (JOBR_DEPTH - 1))

static int sec_ram_setup(struct sec_info *sec_info)
{
	sec_info->phy_addr = (phy_addr_t)bsp_find_phyaddr_from_tbl((unsigned char *)"sec",0);

	if (!sec.phy_addr) {
		printf("No registe memory\n");
		return BSP_ERROR;
	}

	sec_info->virt_addr = bsp_shm_p2v((unsigned long)sec_info->phy_addr);

	memset(sec_info->virt_addr, 0, 0x100000);

	return 0;
}

static int caam_reset_hw_jr(struct sec_info *sec_info)
{
	t_SecJqRegs *regs = sec_info->regs;
	unsigned int timeout = 100000;
	uint64_t cfgr;

	/*
	 * mask interrupts since we are going to poll
	 * for reset completion status
	 */
	cfgr = BSP_GET_BIT64(regs->cfgr);
	cfgr |= 1;
	BSP_WRITE_BIT64(regs->cfgr, cfgr);

	/* initiate flush (required prior to reset) */
	BSP_WRITE_BIT32(regs->cmdr, CMDR_RESET);

	while (((BSP_GET_BIT32(regs->istar) & JRINT_ERR_HALT_MASK) ==
				JRINT_ERR_HALT_INPROGRESS) && --timeout)
		cpu_relax();

	if ((BSP_GET_BIT32(regs->istar) & JRINT_ERR_HALT_MASK) !=
			JRINT_ERR_HALT_COMPLETE || timeout == 0) {
		printf("failed to flush job ring 0\n");
		return BSP_ERROR;
	}

	/* initiate reset */
	timeout = 100000;
	BSP_WRITE_BIT32(regs->cmdr, JRCR_RESET);
	while ((BSP_GET_BIT32(regs->cmdr) & JRCR_RESET) && --timeout)
		cpu_relax();

	if (timeout == 0) {
		printf("failed to reset job ring 0\n");
		return BSP_ERROR;
	}

	return BSP_OK;
}

/*
 * Init JobR independent of platform property detection
 */
int sec_hw_init(struct sec_info *sec_info)
{
	int i, error;
	uint64_t cfgr;
	uint32_t mcfgr;
	struct sec_ram_map *map;
	struct sec_jr *i_ring, *o_ring;
	t_SecJqRegs *regs;
	t_SecGenRegs *gen;

	regs = &p_SecJq0MemMap->jq;
	gen = &p_SecMemMap->gen;

	sec_info->regs = regs;
	sec_info->gen = gen;

	sec_ram_setup(sec_info);

	map = (struct sec_ram_map *)(sec_info->virt_addr);

	error = caam_reset_hw_jr(sec_info);
	if (error)
		return error;

	i_ring = &sec_info->i_ring;
	o_ring = &sec_info->o_ring;

	i_ring->head = i_ring->tail = 0;
	o_ring->head = o_ring->tail = 0;

	mcfgr  = BSP_GET_BIT32(gen->mcfgr);
	BSP_WRITE_BIT32(gen->mcfgr, mcfgr & ~MCFGR_PS);
	mcfgr  = BSP_GET_BIT32(gen->mcfgr);

	BSP_WRITE_BIT64(regs->irbar, __pa(map->i_ring_entry));
	BSP_WRITE_BIT64(regs->orbar, __pa(map->o_ring_entry));

	BSP_WRITE_BIT32(regs->irsr, JOBR_DEPTH);
	BSP_WRITE_BIT32(regs->orsr, JOBR_DEPTH);

	spin_lock_init(&i_ring->lock);
	spin_lock_init(&o_ring->lock);

	cfgr = BSP_GET_BIT64(regs->cfgr);
	cfgr |= 1;
	BSP_WRITE_BIT64(regs->cfgr, cfgr);


	return 0;
}

static int caam_jr_enqueue(struct sec_info *sec_info, uint8_t *desc)
{
	struct sec_ram_map *map = sec_info->virt_addr;
	phy_addr_t desc_dma = __pa(desc);
	int tail;

	spin_lock(&sec_info->i_ring.lock);

	tail = sec_info->i_ring.tail;

	if (!BSP_GET_BIT32(sec_info->regs->irsar)) {
		spin_unlock(&sec_info->i_ring.lock);
		return BSP_ERROR;
	}

	BSP_WRITE_BIT32(map->i_ring_entry[tail][0], desc_dma);

	sec_info->i_ring.tail = (tail + 1) & (JOBR_DEPTH - 1);

	BSP_WRITE_BIT32(sec_info->regs->irjar, 1);

	spin_unlock(&sec_info->i_ring.lock);

	return 0;
}

/* Deferred service handler, run as interrupt-fired tasklet */
static int caam_jr_dequeue(struct sec_info *sec_info)
{
	int cleaned = 0, timeout = 100000;
	uint32_t head;

	while (!BSP_GET_BIT32(sec_info->regs->orsfr) && timeout--);

	if (timeout == 0)
		return -1;

	head = sec_info->o_ring.head;

	spin_lock(&sec_info->o_ring.lock);

	BSP_WRITE_BIT32(sec_info->regs->orjrr, 1);

	sec_info->o_ring.head = OUTPUT_INC(head) & (JOBR_DEPTH - 1);

	spin_unlock(&sec_info->o_ring.lock);

	return cleaned;
}

static void dump_info(u8 *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("0x%02x, ", buf[i]);
	printf("\n");
}

int sec_crypto(struct sec_info *sec_info, struct crypto_info *info)
{
	u16 desc_size = MAX_CAAM_DESCSIZE;
	int ret = BSP_OK;
	uint32_t *desc_buf = &sec_info->virt_addr->desc[info->index];

	switch (info->crypto_type) {
	case ENCRYPTION:
		switch (info->alg_type) {
		case EEA2:
			ret = cnstr_jobdesc_aes_ctr(desc_buf, &desc_size,
					(uint8_t *)info->key, info->keysize,
					info->dir, (uint32_t *)info->iv,
					__pa((uint8_t *)info->src), __pa((uint8_t *)info->dst),
					info->len);

			if (ret) {
				printf("construct descriptor failed\n");
				return ret;
			}
			if (desc_size > MAX_CAAM_DESCSIZE) {
				printf("ERROR: desc size large than \
						MAX_CAAM_DESCSIZE\n");
				return BSP_ERROR;
			}
			break;
		case EEA1:
			ret = cnstr_jobdesc_snow_f8(desc_buf, &desc_size,
					info->key, info->keysize, info->dir,
					info->iv, __pa((uint8_t *)info->src), __pa((uint8_t *)info->dst),
					info->len);
			if (ret) {
				printf("construct descriptor failed\n");
				return ret;
			}

			if (desc_size > MAX_CAAM_DESCSIZE) {
				printf("ERROR: desc size large than \
						MAX_CAAM_DESCSIZE\n");
				return BSP_ERROR;
			}
			break;

		case EEA0:
			printf("Null ciphering algorithm\n");

			break;
		default:
			printf("invalid ciphering algorithm\n");

			return BSP_ERROR;
		}
		break;
	case INTEGRITY:
		switch (info->alg_type) {
		case EIA0:
			break;
		case EIA1:
			ret = cnstr_jobdesc_snow_f9(desc_buf, &desc_size,
					info->key, info->keysize, info->dir,
					info->iv, __pa((uint8_t *)info->src), info->len,
					__pa((uint8_t *)info->dst));
			if (ret) {
				printf("construct descriptor failed\n");
				return ret;
			}

			if (desc_size > MAX_CAAM_DESCSIZE) {
				printf("ERROR: desc size large than \
						MAX_CAAM_DESCSIZE\n");
				return BSP_ERROR;
			}
				
			break;
		case EIA2:
			ret = cnstr_jobdesc_aes_cmac(desc_buf, &desc_size,
					info->key, info->keysize, info->dir,
					__pa((uint8_t *)info->src), info->len, __pa((uint8_t *)info->dst));
			if (ret) {
				printf("construct descriptor failed\n");
				return ret;
			}

			if (desc_size > MAX_CAAM_DESCSIZE) {
				printf("ERROR: desc size large than \
						MAX_CAAM_DESCSIZE\n");
				return BSP_ERROR;
			}
	
			break;
		//default:
		}
		break;
	default:
		printf("error crypto type\n");
	}

	ret = caam_jr_enqueue(sec_info, desc_buf);
	if (ret != BSP_OK)
		return ret;

	ret = caam_jr_dequeue(sec_info);
	if (ret != BSP_OK)
		return ret;

	return ret;
}

int hw_encrypt_engine(uint8_t *src, uint8_t *dst, uint32_t len,
		uint32_t count_c, uint8_t bearer, uint8_t direction,
		uint32_t algrithm, uint8_t *key)
{
	struct crypto_info info;
	int ret;
	uint32_t *iv;

	info.src = src;
	info.key = key;
	info.dst = dst;

	info.len = len;
	info.keysize = 16 * 8;
	info.crypto_type = ENCRYPTION;
	info.alg_type = algrithm;
	info.dir = DIR_ENCRYPT;

	info.index = JOB_INC(sec.buf_index);
	iv = sec.virt_addr->iv[info.index];

	iv[0] = count_c;
	iv[1] = (bearer << 27) | (direction << 26);
	iv[2] = 0;
	iv[3] = 0;

	info.iv = iv;

	ret = sec_crypto(&sec, &info);
	if (ret)
		return ret;

	return 0;
}

int hw_decrypt_engine(uint8_t *src, uint8_t *dst, uint32_t len,
		uint32_t count_c, uint8_t bearer, uint8_t direction,
		uint32_t algrithm, uint8_t *key)
{
	struct crypto_info info;
	int ret;
	uint32_t *iv;

	info.len = len;
	info.keysize = 16 * 8;
	info.crypto_type = ENCRYPTION;
	info.alg_type = algrithm;
	info.dir = DIR_DECRYPT;

	info.src = src;
	info.key = key;
	info.dst = dst;

	info.index = JOB_INC(sec.buf_index);
	iv = sec.virt_addr->iv[info.index];

	iv[0] = count_c;
	iv[1] = (bearer << 27) | (direction << 26);
	iv[2] = 0;
	iv[3] = 0;

	info.iv = iv;

	ret = sec_crypto(&sec, &info);
	if (ret)
		return ret;

	return 0;
}

int bsp_hw_integrity_snow3g_f9(uint8_t *key, uint32_t count, uint32_t fresh,
		uint32_t direction, uint8_t *data, uint32_t len, uint8_t *mac)
{
	struct crypto_info info;
	int ret;
	uint32_t *iv;

	info.len = len;
	info.keysize = 16 * 8;
	info.crypto_type = INTEGRITY;
	info.alg_type = EIA1;
	info.dir = DIR_ENCRYPT;

	info.src = data;
	info.key = key;

	info.index = JOB_INC(sec.buf_index);
	iv = sec.virt_addr->iv[info.index];
	info.dst = sec.virt_addr->dst[info.index];

	iv[0] = count;
	iv[1] = direction << 26;
	iv[2] = fresh << 27;
	iv[3] = 0;

	info.iv = iv;

	ret = sec_crypto(&sec, &info);
	if (ret)
		return ret;

	*(uint32_t *)mac = *(uint32_t *)info.dst;

	return 0;
}

int bsp_hw_integrity_aes_cmac(uint8_t *key, uint32_t count, uint32_t bearer,
		uint32_t direction, uint8_t *data, uint32_t len, uint8_t *mac)
{
	struct crypto_info info;
	int ret;
	uint32_t *src;

	info.len = len + 8;
	info.keysize = 16 * 8;
	info.crypto_type = INTEGRITY;
	info.alg_type = EIA2;
	info.dir = DIR_ENCRYPT;

	info.key = key;

	info.index = JOB_INC(sec.buf_index);

	src = (uint32_t *)sec.virt_addr->src[info.index];
	info.dst = sec.virt_addr->dst[info.index];

	src[0] = count;
	src[1] = (bearer << 27) | (direction << 26);

	memcpy(&src[2], data, len);

	info.src = (uint8_t *)src;

	ret = sec_crypto(&sec, &info);
	if (ret)
		return ret;

	*(uint32_t *)mac = *(uint32_t *)info.dst;

	return 0;
}

static uint8_t aes_cmac_src[] = {0x00, 0x48, 0x01, 0xa4, 0xea,0x05, 0xce, 0x4a, 0xa0, 0x91, 0xe5, 0xb2, 0xed, 0x6b, 0xc7, 0x9b, 0xc0};
static uint8_t aes_cmac_key[] = {0xb4, 0xd0, 0xdf, 0x3a, 0x85, 0x01, 0x69, 0x95, 0x5d, 0x2e, 0xa1, 0xdc, 0x41, 0xa6, 0xcf, 0xe2};
static uint8_t aes_cmac_mac[4];

static uint8_t aes_key[MAX_BUFFER_SIZE] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
static uint8_t aes_src[MAX_BUFFER_SIZE] = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a};
static uint8_t aes_dst[MAX_BUFFER_SIZE];

static uint8_t snow3g_key[] = {0x2B, 0xD6, 0x45, 0x9F, 0x82, 0xC5, 0xB3, 0x00, 0x95, 0x2C, 0x49, 0x10, 0x48, 0x81, 0xFF, 0x48};
static uint32_t snow3g_src[MAX_BUFFER_SIZE] = {
 	0x7EC61272, 0x743BF161, 0x4726446A, 0x6C38CED1,
 	0x66F6CA76, 0xEB543004, 0x4286346C, 0xEF130F92,
 	0x922B0345, 0x0D3A9975, 0xE5BD2EA0, 0xEB55AD8E,
 	0x1B199E3E, 0xC4316020, 0xE9A1B285, 0xE7627953,
};
static uint32_t snow3g_dst[MAX_BUFFER_SIZE];

void aes_integrity_test(void)
{
	int ret;
	uint8_t result[4] = {0x25, 0xbb, 0x8a, 0xc9};
	uint8_t *src = XOS_GETUB(sizeof(aes_cmac_src));
	uint8_t *dst = XOS_GETUB(sizeof(aes_cmac_mac));
	uint8_t *key = XOS_GETUB(sizeof(aes_cmac_key));

	memset(dst, 0, sizeof(aes_cmac_mac));

	memcpy(src, aes_cmac_src, sizeof(aes_cmac_src));
	memcpy(key, aes_cmac_key, sizeof(aes_cmac_key));

	ret = bsp_hw_integrity_aes_cmac((uint8_t *)key, 0, 1,
			0, (uint8_t *)src, 17, dst);
	if (ret) {
		printf("Test failed\n");
		return;
	}

	memcpy(aes_cmac_mac, dst, 4);
	printf("0x%08x 0x%08x\n", *(uint32_t *)result, *(uint32_t *)aes_cmac_mac);
}

void snow3g_integrity_test(void)
{
	int ret;
	uint8_t result[4] = {0x3f, 0xc9, 0x18, 0x97};
	uint8_t *src = XOS_GETUB(sizeof(aes_cmac_src));
	uint8_t *dst = XOS_GETUB(sizeof(aes_cmac_mac));
	uint8_t *key = XOS_GETUB(sizeof(aes_cmac_key));

	memset(dst, 0, sizeof(aes_cmac_mac));

	memcpy(src, aes_cmac_src, sizeof(aes_cmac_src));
	memcpy(key, aes_cmac_key, sizeof(aes_cmac_key));

	ret = bsp_hw_integrity_snow3g_f9((uint8_t *)key, 0, 1,
			0, (uint8_t *)src, 17, dst);
	if (ret) {
		printf("Test failed\n");
		return;
	}

	memcpy(aes_cmac_mac, dst, 4);
	printf("0x%08x 0x%08x\n", *(uint32_t *)result, *(uint32_t *)aes_cmac_mac);
}

void aes_crypto_test(void)
{
	int ret;
	uint8_t result[16] = {0x1f, 0x6f, 0x9f, 0x62, 0xf2, 0xc7, 0xe8, 0x54, 0x62, 0x53, 0x7c, 0xba, 0xe7, 0xbc, 0x71, 0x81};
	uint8_t *src = XOS_GETUB(sizeof(aes_src));
	uint8_t *dst = XOS_GETUB(sizeof(aes_dst));
	uint8_t *key = XOS_GETUB(sizeof(aes_key));

	memset(dst, 0, sizeof(aes_dst));

	memcpy(src, aes_src, sizeof(aes_src));
	memcpy(key, aes_key, sizeof(aes_key));

	ret = hw_encrypt_engine(src, dst, 16, 0, 1, 0, EEA1, key);
	if (ret) {
		printf("Test failed\n");
		return;
	}

	{

		int i;
		uint8_t *buf = dst;
		for (i = 0; i < 16; i++)
			printf("0x%02x, ", buf[i]);
		printf("\n");

		buf = result;
		for (i = 0; i < 16; i++)
			printf("0x%02x, ", buf[i]);
		printf("\n");

	}
}

void snow3g_crypto_test(void)
{
	int ret;

	uint8_t result[16] = {0x8b, 0x1d, 0x6a, 0xf2, 0x99, 0xfd, 0x8d, 0x0b, 0x8f, 0x0d, 0x71, 0x6b, 0x22, 0xa7, 0x35, 0x67};
	uint8_t *src = XOS_GETUB(sizeof(aes_src));
	uint8_t *dst = XOS_GETUB(sizeof(aes_dst));
	uint8_t *key = XOS_GETUB(sizeof(aes_key));

	memset(dst, 0, sizeof(aes_dst));

	memcpy(src, aes_src, sizeof(aes_src));
	memcpy(key, aes_key, sizeof(aes_key));

	ret = hw_encrypt_engine(src, dst, 16, 0, 1, 0, EEA2, key);
	if (ret) {
		printf("Test failed\n");
		return;
	}

	{

		int i;
		uint8_t *buf = dst;
		for (i = 0; i < 16; i++)
			printf("0x%02x, ", buf[i]);
		printf("\n");

		buf = result;
		for (i = 0; i < 16; i++)
			printf("0x%02x, ", buf[i]);
		printf("\n");
	}
}

static void sec_encrypt_performance_test(int size, int algm)
{
	uint8_t *src = XOS_GETUB(sizeof(aes_src));
	uint8_t *dst = XOS_GETUB(sizeof(aes_src));
	uint8_t *key = XOS_GETUB(sizeof(aes_key));
	uint64_t l[2], i;

	memset(dst, 0, sizeof(aes_src));

	if (algm == EEA2) {
		printf("###############aes###############################\n");
		memcpy(src, aes_src, sizeof(aes_src));
		memcpy(key, aes_key, sizeof(aes_key));
	} else if (algm == EEA1) {
		printf("###############snow3G f8 ###############################\n");
		memcpy(src, snow3g_src, sizeof(snow3g_src));
		memcpy(key, snow3g_key, sizeof(snow3g_key));
	} else
		return

	l[0] = mfatb();
	l[1] = mfatb();

	l[0] = mfatb();

	i = 0;
	for (i = 0; i< 1000; i++)
	if (hw_encrypt_engine(src, dst, size, 1, 0, 0, algm, key))
		return;

	l[1] = mfatb();

	printf("%d ***%lld us\n", size, (l[1] - l[0]) / 1200000);
}

void bsp_sec_test(void)
{
	uint8_t *src = XOS_GETUB(sizeof(aes_cmac_src));
	uint8_t *dst = XOS_GETUB(sizeof(aes_cmac_mac));
	uint8_t *key = XOS_GETUB(sizeof(aes_cmac_key));
	int ret;

	uint8_t *src1 = XOS_GETUB(sizeof(aes_src));
	uint8_t *dst1 = XOS_GETUB(sizeof(aes_dst));
	uint8_t *key1 = XOS_GETUB(sizeof(aes_key));

	memcpy(src, aes_cmac_src, sizeof(aes_cmac_src));
	memcpy(key, aes_cmac_key, sizeof(aes_cmac_key));

	memcpy(src1, aes_src, sizeof(aes_src));
	memcpy(key1, aes_key, sizeof(aes_key));

	while (1) {
		{
			uint8_t result[4] = {0x25, 0xbb, 0x8a, 0xc9};
			memset(dst, 0, sizeof(aes_cmac_mac));
			ret = bsp_hw_integrity_aes_cmac((uint8_t *)key, 0, 1,
					0, (uint8_t *)src, 17, dst);
			if (ret) {
				printf("Test failed\n");
				return;
			}

			if (memcmp(dst, result, 4) != 0)
				printf("diff %x %x\n", *(uint32_t *)dst, *(uint32_t *)result);
		}

		{

			uint8_t result[4] = {0x3f, 0xc9, 0x18, 0x97};
			memset(dst, 0, sizeof(aes_cmac_mac));
			ret = bsp_hw_integrity_snow3g_f9((uint8_t *)key, 0, 1,
					0, (uint8_t *)src, 17, dst);
			if (ret) {
				printf("Test failed\n");
				return;
			}

			if (memcmp(dst, result, 4) != 0)
				printf("diff %x %x\n", *(uint32_t *)dst, *(uint32_t *)result);
		}

		{
			uint8_t result[16] = {0x1f, 0x6f, 0x9f, 0x62, 0xf2, 0xc7, 0xe8, 0x54, 0x62, 0x53, 0x7c, 0xba, 0xe7, 0xbc, 0x71, 0x81};
			memset(dst1, 0, sizeof(aes_dst));

			ret = hw_encrypt_engine(src1, dst1, 16, 0, 1, 0, EEA1, key1);
			if (ret) {
				printf("Test failed\n");
				return;
			}

			{
				int i;
				uint8_t *buf = dst1;
				for (i = 0; i < 16; i++)
					if (buf[i] != result[i])
						printf("%d diff[%d] 0x%02x != 0x%02x\n", __LINE__, i, buf[i], result[i]);
			}
		}

		{
			uint8_t result[16] = {0x8b, 0x1d, 0x6a, 0xf2, 0x99, 0xfd, 0x8d, 0x0b, 0x8f, 0x0d, 0x71, 0x6b, 0x22, 0xa7, 0x35, 0x67};
			memset(dst1, 0, sizeof(aes_dst));

			ret = hw_encrypt_engine(src1, dst1, 16, 0, 1, 0, EEA2, key1);
			if (ret) {
				printf("Test failed\n");
				return;
			}

			{
				int i;
				uint8_t *buf = dst1;
				for (i = 0; i < 16; i++)
					if (buf[i] != result[i])
						printf("%d diff[%d] 0x%02x != 0x%02x\n", __LINE__, i, buf[i], result[i]);
			}

		}
	}

	
}

/**********************************************************************
 * 函数名称：BspSecModuleInit
 * 功能描述：SEC初始化
 * 访问的表：无
 * 修改的表：无
 * 输入参数：
 * 	                      无
 * 输出参数：
 * 返 回 值：
 *			0:成功
 *               其它:失败
 * 其它说明：
 * 修改日期      版本号  修改人      修改内容
 * ---------------------------------------------------------------------
 *          
 ************************************************************************/
ULONG bsp_sec_init(void)
{   
	ULONG  ret = BSP_OK;
	ret = BspDpaShmemSetup();
	if (ret) {
		printf("Share memory setup failed\n");
		return BSP_ERROR;
	}
	ret = bsp_sec_reg_init();

	if(BSP_ERROR == ret)
	{
		printf("bsp_sec_reg_init Failed!%s failed, in file:%s, on line:%d, ret = %d\n", __FUNCTION__, __FILE__, __LINE__, (int)ret);
		return BSP_ERROR;
	}
	sec_hw_init(&sec);
	return BSP_OK;
}


