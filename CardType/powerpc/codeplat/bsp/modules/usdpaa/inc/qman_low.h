/* Copyright (c) 2008-2010 Freescale Semiconductor, Inc.
 * All rights reserved.
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

#include "qman_private.h"

/***************************/
/* Portal register assists */
/***************************/

/* Cache-inhibited register offsets */
#define REG_EQCR_PI_CINH	(void *)0x0000
#define REG_EQCR_CI_CINH	(void *)0x0004
#define REG_EQCR_ITR		(void *)0x0008
#define REG_DQRR_PI_CINH	(void *)0x0040
#define REG_DQRR_CI_CINH	(void *)0x0044
#define REG_DQRR_ITR		(void *)0x0048
#define REG_DQRR_DCAP		(void *)0x0050
#define REG_DQRR_SDQCR		(void *)0x0054
#define REG_DQRR_VDQCR		(void *)0x0058
#define REG_DQRR_PDQCR		(void *)0x005c
#define REG_MR_PI_CINH		(void *)0x0080
#define REG_MR_CI_CINH		(void *)0x0084
#define REG_MR_ITR		(void *)0x0088
#define REG_CFG			(void *)0x0100
#define REG_ISR			(void *)0x0e00
#define REG_ITPR		(void *)0x0e14

/* Cache-enabled register offsets */
#define CL_EQCR			(void *)0x0000
#define CL_DQRR			(void *)0x1000
#define CL_MR			(void *)0x2000
#define CL_EQCR_PI_CENA		(void *)0x3000
#define CL_EQCR_CI_CENA		(void *)0x3100
#define CL_DQRR_PI_CENA		(void *)0x3200
#define CL_DQRR_CI_CENA		(void *)0x3300
#define CL_MR_PI_CENA		(void *)0x3400
#define CL_MR_CI_CENA		(void *)0x3500
#define CL_CR			(void *)0x3800
#define CL_RR0			(void *)0x3900
#define CL_RR1			(void *)0x3940

/* The h/w design requires mappings to be size-aligned so that "add"s can be
 * reduced to "or"s. The primitives below do the same for s/w. */

/* Bitwise-OR two pointers */
static inline void *ptr_OR(void *a, void *b)
{
	return (void *)((unsigned long)a | (unsigned long)b);
}

/* Cache-inhibited register access */
static inline u32 __qm_in(struct qm_addr *qm, void *offset)
{
    u32 ret;
    //printf("&qm->addr_ci's addr-->0x%lx\n",qm->addr_ci);
    //printf("qm->addr_ci's value=0x%lx\n",*(unsigned int *)qm->addr_ci);
    //while(1);
	return in_be32(ptr_OR(qm->addr_ci, offset));
}
static inline void __qm_out(struct qm_addr *qm, void *offset, u32 val)
{
	out_be32(ptr_OR(qm->addr_ci, offset), val);
}
#define qm_in(reg)		__qm_in(&portal->addr, REG_##reg)
#define qm_out(reg, val)	__qm_out(&portal->addr, REG_##reg, val)

/* Convert 'n' cachelines to a pointer value for bitwise OR */
#define qm_cl(n)		(void *)((n) << 6)

/* Cache-enabled (index) register access */
static inline void __qm_cl_touch_ro(struct qm_addr *qm, void *offset)
{
	dcbt_ro(ptr_OR(qm->addr_ce, offset));
}
static inline void __qm_cl_touch_rw(struct qm_addr *qm, void *offset)
{
	dcbt_rw(ptr_OR(qm->addr_ce, offset));
}
static inline u32 __qm_cl_in(struct qm_addr *qm, void *offset)
{
	return in_be32(ptr_OR(qm->addr_ce, offset));
}
static inline void __qm_cl_out(struct qm_addr *qm, void *offset, u32 val)
{
	out_be32(ptr_OR(qm->addr_ce, offset), val);
	dcbf(ptr_OR(qm->addr_ce, offset));
}
static inline void __qm_cl_invalidate(struct qm_addr *qm, void *offset)
{
	dcbi(ptr_OR(qm->addr_ce, offset));
}
#define qm_cl_touch_ro(reg)	__qm_cl_touch_ro(&portal->addr, CL_##reg##_CENA)
#define qm_cl_touch_rw(reg)	__qm_cl_touch_rw(&portal->addr, CL_##reg##_CENA)
#define qm_cl_in(reg)		__qm_cl_in(&portal->addr, CL_##reg##_CENA)
#define qm_cl_out(reg, val)	__qm_cl_out(&portal->addr, CL_##reg##_CENA, val)
#define qm_cl_invalidate(reg) __qm_cl_invalidate(&portal->addr, CL_##reg##_CENA)

/* Cyclic helper for rings. FIXME: once we are able to do fine-grain perf
 * analysis, look at using the "extra" bit in the ring index registers to avoid
 * cyclic issues. */
static inline u8 cyc_diff(u8 ringsize, u8 first, u8 last)
{
	/* 'first' is included, 'last' is excluded */
	if (first <= last)
		return last - first;
	return ringsize + last - first;
}

/* Inlining (and/or loops) can go horribly wrong if the compiler caches
 * foo->verb - it doesn't realise that dcbi()s and dcbt()s mean that a new
 * value will show eventually up (it assumes coherency). Use this accessor to
 * read verb bytes to address this issue. */
static inline u8 read_verb(volatile void *p)
{
	volatile u8 *__p = p;
	return *__p;
}


/* ---------------- */
/* --- EQCR API --- */

/* Bit-wise logic to wrap a ring pointer by clearing the "carry bit" */
#define EQCR_CARRYCLEAR(p) \
	(void *)((unsigned long)(p) & (~(unsigned long)(QM_EQCR_SIZE << 6)))

/* Bit-wise logic to convert a ring pointer to a ring index */
static inline u8 EQCR_PTR2IDX(struct qm_eqcr_entry *e)
{
	return ((u32)e >> 6) & (QM_EQCR_SIZE - 1);
}

/* Increment the 'cursor' ring pointer, taking 'vbit' into account */
static inline void EQCR_INC(struct qm_eqcr *eqcr)
{
	/* NB: this is odd-looking, but experiments show that it generates fast
	 * code with essentially no branching overheads. We increment to the
	 * next EQCR pointer and handle overflow and 'vbit'. */
	struct qm_eqcr_entry *partial = eqcr->cursor + 1;
	eqcr->cursor = EQCR_CARRYCLEAR(partial);
	if (partial != eqcr->cursor)
		eqcr->vbit ^= QM_EQCR_VERB_VBIT;
}

static inline int qm_eqcr_init(struct qm_portal *portal, enum qm_eqcr_pmode pmode,
		__maybe_unused enum qm_eqcr_cmode cmode)
{
	/* This use of 'register', as well as all other occurances, is because
	 * it has been observed to generate much faster code with gcc than is
	 * otherwise the case. */
	register struct qm_eqcr *eqcr = &portal->eqcr;
	u32 cfg;
	u8 pi;
	if (__qm_portal_bind(portal, QM_BIND_EQCR))
	{
		return -EBUSY;
	}
	eqcr->ring = ptr_OR(portal->addr.addr_ce, CL_EQCR);

	eqcr->ci = qm_in(EQCR_CI_CINH) & (QM_EQCR_SIZE - 1);
	//printf("000000000000000000000000\n");
	qm_cl_invalidate(EQCR_CI);
	//printf("11111111111111111111\n");

	pi = qm_in(EQCR_PI_CINH) & (QM_EQCR_SIZE - 1);
	eqcr->cursor = eqcr->ring + pi;
	eqcr->vbit = (qm_in(EQCR_PI_CINH) & QM_EQCR_SIZE) ?
			QM_EQCR_VERB_VBIT : 0;
	eqcr->available = QM_EQCR_SIZE - 1 -
			cyc_diff(QM_EQCR_SIZE, eqcr->ci, pi);
	eqcr->ithresh = qm_in(EQCR_ITR);

  
 

#ifdef CONFIG_FSL_QMAN_CHECKING
	eqcr->busy = 0;
	eqcr->pmode = pmode;
	eqcr->cmode = cmode;
#endif
	cfg = (qm_in(CFG) & 0x00ffffff) |
		((pmode & 0x3) << 24);	/* QCSP_CFG::EPM */
	qm_out(CFG, cfg);
	
	
	return 0;
}

static inline void qm_eqcr_finish(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	u8 pi = qm_in(EQCR_PI_CINH) & (QM_EQCR_SIZE - 1);
	u8 ci = qm_in(EQCR_CI_CINH) & (QM_EQCR_SIZE - 1);

	QM_ASSERT(!eqcr->busy);
	if (pi != EQCR_PTR2IDX(eqcr->cursor))
		pr_crit("losing uncommited EQCR entries\n");
	if (ci != eqcr->ci)
		pr_crit("missing existing EQCR completions\n");
	if (eqcr->ci != EQCR_PTR2IDX(eqcr->cursor))
		pr_crit("EQCR destroyed unquiesced\n");
	__qm_portal_unbind(portal, QM_BIND_EQCR);
}

static inline struct qm_eqcr_entry *qm_eqcr_start(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	QM_ASSERT(!eqcr->busy);
	if (!eqcr->available)
		return NULL;
#ifdef CONFIG_FSL_QMAN_CHECKING
	eqcr->busy = 1;
#endif
	dcbzl(eqcr->cursor);
	return eqcr->cursor;
}

static inline void qm_eqcr_abort(struct qm_portal *portal)
{
	__maybe_unused register struct qm_eqcr *eqcr = &portal->eqcr;
	QM_ASSERT(eqcr->busy);
#ifdef CONFIG_FSL_QMAN_CHECKING
	eqcr->busy = 0;
#endif
}

static inline struct qm_eqcr_entry *qm_eqcr_pend_and_next(struct qm_portal *portal, u8 myverb)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	QM_ASSERT(eqcr->busy);
	QM_ASSERT(eqcr->pmode != qm_eqcr_pvb);
	if (eqcr->available == 1)
		return NULL;
	eqcr->cursor->__dont_write_directly__verb = myverb | eqcr->vbit;
	dcbf(eqcr->cursor);
	EQCR_INC(eqcr);
	eqcr->available--;
	dcbzl(eqcr->cursor);
	return eqcr->cursor;
}

#define EQCR_COMMIT_CHECKS(eqcr) \
do { \
	QM_ASSERT(eqcr->busy); \
	QM_ASSERT(eqcr->cursor->orp == (eqcr->cursor->orp & 0x00ffffff)); \
	QM_ASSERT(eqcr->cursor->fqid == (eqcr->cursor->fqid & 0x00ffffff)); \
} while(0)

#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1
static inline void eqcr_fd_fixup(struct qm_eqcr_entry *eq)
{
	if (qman_ip_rev == QMAN_REV1) {
		struct qm_fd *fd = &eq->fd;
		/* The struct's address went from 48-bit to 40-bit but rev1
		 * chips will still interpret it as 48-bit, meaning we have to
		 * scrub the upper 8-bits in that case, just in case the user
		 * left noise in there. NB, this code is the most explicit but
		 * if the compiler doesn't optimise it (they are 4-bits each of
		 * the same byte), it may be more efficient to do;
		 *    ((u8 *)fd)[2] = 0;
		 */
		fd->eliodn_offset = 0;
		fd->__reserved = 0;
	}
}
#else
#define eqcr_fd_fixup(fd) do { ; } while (0)
#endif

static inline void qm_eqcr_pci_commit(struct qm_portal *portal, u8 myverb)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	EQCR_COMMIT_CHECKS(eqcr);
	QM_ASSERT(eqcr->pmode == qm_eqcr_pci);
	eqcr_fd_fixup(eqcr->cursor);
	eqcr->cursor->__dont_write_directly__verb = myverb | eqcr->vbit;
	EQCR_INC(eqcr);
	eqcr->available--;
	dcbf(eqcr->cursor);
	hwsync();
	qm_out(EQCR_PI_CINH, EQCR_PTR2IDX(eqcr->cursor));
#ifdef CONFIG_FSL_QMAN_CHECKING
	eqcr->busy = 0;
#endif
}

static inline void qm_eqcr_pce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_eqcr *eqcr = &portal->eqcr;
	QM_ASSERT(eqcr->pmode == qm_eqcr_pce);
	qm_cl_invalidate(EQCR_PI);
	qm_cl_touch_rw(EQCR_PI);
}

static inline void qm_eqcr_pce_commit(struct qm_portal *portal, u8 myverb)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	EQCR_COMMIT_CHECKS(eqcr);
	QM_ASSERT(eqcr->pmode == qm_eqcr_pce);
	eqcr_fd_fixup(eqcr->cursor);
	eqcr->cursor->__dont_write_directly__verb = myverb | eqcr->vbit;
	EQCR_INC(eqcr);
	eqcr->available--;
	dcbf(eqcr->cursor);
	lwsync();
	qm_cl_out(EQCR_PI, EQCR_PTR2IDX(eqcr->cursor));
#ifdef CONFIG_FSL_QMAN_CHECKING
	eqcr->busy = 0;
#endif
}

static inline void qm_eqcr_pvb_commit(struct qm_portal *portal, u8 myverb)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	struct qm_eqcr_entry *eqcursor;
	EQCR_COMMIT_CHECKS(eqcr);
	QM_ASSERT(eqcr->pmode == qm_eqcr_pvb);
	eqcr_fd_fixup(eqcr->cursor);
	lwsync();
	eqcursor = eqcr->cursor;
	eqcursor->__dont_write_directly__verb = myverb | eqcr->vbit;
	dcbf(eqcursor);
	EQCR_INC(eqcr);
	eqcr->available--;
#ifdef CONFIG_FSL_QMAN_CHECKING
	eqcr->busy = 0;
#endif
}

static inline u8 qm_eqcr_cci_update(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	u8 diff, old_ci = eqcr->ci;
	QM_ASSERT(eqcr->cmode == qm_eqcr_cci);
	eqcr->ci = qm_in(EQCR_CI_CINH) & (QM_EQCR_SIZE - 1);
	diff = cyc_diff(QM_EQCR_SIZE, old_ci, eqcr->ci);
	eqcr->available += diff;
	return diff;
}

static inline void qm_eqcr_cce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_eqcr *eqcr = &portal->eqcr;
	QM_ASSERT(eqcr->cmode == qm_eqcr_cce);
	qm_cl_touch_ro(EQCR_CI);
}

static inline u8 qm_eqcr_cce_update(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	u8 diff, old_ci = eqcr->ci;
	QM_ASSERT(eqcr->cmode == qm_eqcr_cce);
	eqcr->ci = qm_cl_in(EQCR_CI) & (QM_EQCR_SIZE - 1);
	qm_cl_invalidate(EQCR_CI);
	diff = cyc_diff(QM_EQCR_SIZE, old_ci, eqcr->ci);
	eqcr->available += diff;
	return diff;
}

static inline u8 qm_eqcr_get_ithresh(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	return eqcr->ithresh;
}

static inline void qm_eqcr_set_ithresh(struct qm_portal *portal, u8 ithresh)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	eqcr->ithresh = ithresh;
	qm_out(EQCR_ITR, ithresh);
}

static inline u8 qm_eqcr_get_avail(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	return eqcr->available;
}

static inline u8 qm_eqcr_get_fill(struct qm_portal *portal)
{
	register struct qm_eqcr *eqcr = &portal->eqcr;
	return QM_EQCR_SIZE - 1 - eqcr->available;
}


/* ---------------- */
/* --- DQRR API --- */

/* FIXME: many possible improvements;
 * - look at changing the API to use pointer rather than index parameters now
 *   that 'cursor' is a pointer,
 * - consider moving other parameters to pointer if it could help (ci)
 */

#define DQRR_CARRYCLEAR(p) \
	(void *)((unsigned long)(p) & (~(unsigned long)(QM_DQRR_SIZE << 6)))

static inline u8 DQRR_PTR2IDX(struct qm_dqrr_entry *e)
{
	return ((u32)e >> 6) & (QM_DQRR_SIZE - 1);
}

static inline struct qm_dqrr_entry *DQRR_INC(struct qm_dqrr_entry *e)
{
	return DQRR_CARRYCLEAR(e + 1);
}

static inline void qm_dqrr_set_maxfill(struct qm_portal *portal, u8 mf)
{
	qm_out(CFG, (qm_in(CFG) & 0xff0fffff) |
		((mf & (QM_DQRR_SIZE - 1)) << 20));
}

static inline int qm_dqrr_init(struct qm_portal *portal, enum qm_dqrr_dmode dmode,
		__maybe_unused enum qm_dqrr_pmode pmode,
		enum qm_dqrr_cmode cmode, u8 max_fill,
		int stash_ring, int stash_data)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	const struct qm_portal_config *config = qm_portal_config(portal);
	u32 cfg;

	if (__qm_portal_bind(portal, QM_BIND_DQRR))
		return -EBUSY;
	if ((stash_ring || stash_data) &&
			((config->cpu == -1) || !config->has_hv_dma))
		return -EINVAL;
	/* Make sure the DQRR will be idle when we enable */
	qm_out(DQRR_SDQCR, 0);
	qm_out(DQRR_VDQCR, 0);
	qm_out(DQRR_PDQCR, 0);
	dqrr->ring = ptr_OR(portal->addr.addr_ce, CL_DQRR);
	dqrr->pi = qm_in(DQRR_PI_CINH) & (QM_DQRR_SIZE - 1);
	dqrr->ci = qm_in(DQRR_CI_CINH) & (QM_DQRR_SIZE - 1);
	dqrr->cursor = dqrr->ring + dqrr->ci;
	dqrr->fill = cyc_diff(QM_DQRR_SIZE, dqrr->ci, dqrr->pi);
	dqrr->vbit = (qm_in(DQRR_PI_CINH) & QM_DQRR_SIZE) ?
			QM_DQRR_VERB_VBIT : 0;
	dqrr->ithresh = qm_in(DQRR_ITR);
#ifdef CONFIG_FSL_QMAN_CHECKING
	dqrr->dmode = dmode;
	dqrr->pmode = pmode;
	dqrr->cmode = cmode;
	dqrr->flags = 0;
	if (stash_ring)
		dqrr->flags |= QM_DQRR_FLAG_RE;
	if (stash_data)
		dqrr->flags |= QM_DQRR_FLAG_SE;
#endif
	cfg = (qm_in(CFG) & 0xff000f00) |
		((max_fill & (QM_DQRR_SIZE - 1)) << 20) | /* DQRR_MF */  /*  ����DQRR��entry�ĸ��� */
		((dmode & 1) << 18) |			/* DP */
		((cmode & 3) << 16) |			/* DCM */
		(stash_ring ? 0x80 : 0) |		/* RE */
		(0 ? 0x40 : 0) |			/* Ignore RP */
		(stash_data ? 0x20 : 0) |		/* SE */
		(0 ? 0x10 : 0);				/* Ignore SP */
       //printf("cfg = 0x%x, in function:%s, on line:%d\n", cfg, __FUNCTION__, __LINE__);
	qm_out(CFG, cfg);
	qm_dqrr_set_maxfill(portal, max_fill);
	return 0;
}

static inline void qm_dqrr_finish(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	if (dqrr->ci != DQRR_PTR2IDX(dqrr->cursor))
		pr_crit("Ignoring completed DQRR entries\n");
	__qm_portal_unbind(portal, QM_BIND_DQRR);
}

static inline void qm_dqrr_current_prefetch(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	/* If ring entries get stashed, don't invalidate/prefetch */
	QM_ASSERT(!(dqrr->flags & QM_DQRR_FLAG_RE));
	dcbt_ro(dqrr->cursor);
}

static inline struct qm_dqrr_entry *qm_dqrr_current(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	if (!dqrr->fill)
		return NULL;
	return dqrr->cursor;
}

static inline u8 qm_dqrr_cursor(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	return DQRR_PTR2IDX(dqrr->cursor);
}

static inline u8 qm_dqrr_next(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->fill);
	dqrr->cursor = DQRR_INC(dqrr->cursor);
	return --dqrr->fill;
}

static inline u8 qm_dqrr_pci_update(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	u8 diff, old_pi = dqrr->pi;
	QM_ASSERT(dqrr->pmode == qm_dqrr_pci);
	dqrr->pi = qm_in(DQRR_PI_CINH) & (QM_DQRR_SIZE - 1);
	diff = cyc_diff(QM_DQRR_SIZE, old_pi, dqrr->pi);
	dqrr->fill += diff;
	return diff;
}

static inline void qm_dqrr_pce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->pmode == qm_dqrr_pce);
	qm_cl_invalidate(DQRR_PI);
	qm_cl_touch_ro(DQRR_PI);
}

static inline u8 qm_dqrr_pce_update(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	u8 diff, old_pi = dqrr->pi;
	QM_ASSERT(dqrr->pmode == qm_dqrr_pce);
	dqrr->pi = qm_cl_in(DQRR_PI) & (QM_DQRR_SIZE - 1);
	diff = cyc_diff(QM_DQRR_SIZE, old_pi, dqrr->pi);
	dqrr->fill += diff;
	return diff;
}

static inline void qm_dqrr_pvb_prefetch(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->pmode == qm_dqrr_pvb);
	QM_ASSERT(!(dqrr->flags & QM_DQRR_FLAG_RE));
	dcbi(ptr_OR(dqrr->ring, qm_cl(dqrr->pi)));
	dcbt_ro(ptr_OR(dqrr->ring, qm_cl(dqrr->pi)));
}

static inline u8 qm_dqrr_pvb_update(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	struct qm_dqrr_entry *res = ptr_OR(dqrr->ring, qm_cl(dqrr->pi));
	QM_ASSERT(dqrr->pmode == qm_dqrr_pvb);
	if ((read_verb(&res->verb) & QM_DQRR_VERB_VBIT) == dqrr->vbit) {
		dqrr->pi = (dqrr->pi + 1) & (QM_DQRR_SIZE - 1);
		if (!dqrr->pi)
			dqrr->vbit ^= QM_DQRR_VERB_VBIT;
		dqrr->fill++;
		return 1;
	}
	return 0;
}

static inline void qm_dqrr_cci_consume(struct qm_portal *portal, u8 num)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cci);
	dqrr->ci = (dqrr->ci + num) & (QM_DQRR_SIZE - 1);
	qm_out(DQRR_CI_CINH, dqrr->ci);
}

static inline void qm_dqrr_cci_consume_to_current(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cci);
	dqrr->ci = DQRR_PTR2IDX(dqrr->cursor);
	qm_out(DQRR_CI_CINH, dqrr->ci);
}

static inline void qm_dqrr_cce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cce);
	qm_cl_invalidate(DQRR_CI);
	qm_cl_touch_rw(DQRR_CI);
}

static inline void qm_dqrr_cce_consume(struct qm_portal *portal, u8 num)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cce);
	dqrr->ci = (dqrr->ci + num) & (QM_DQRR_SIZE - 1);
	qm_cl_out(DQRR_CI, dqrr->ci);
}

static inline void qm_dqrr_cce_consume_to_current(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cce);
	dqrr->ci = DQRR_PTR2IDX(dqrr->cursor);
	qm_cl_out(DQRR_CI, dqrr->ci);
}

static inline void qm_dqrr_cdc_consume_1(struct qm_portal *portal, u8 idx, int park)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cdc);
	QM_ASSERT(idx < QM_DQRR_SIZE);
	qm_out(DQRR_DCAP, (0 << 8) |	/* S */
		((park ? 1 : 0) << 6) |	/* PK */
		idx);			/* DCAP_CI */
}

static inline void qm_dqrr_cdc_consume_1ptr(struct qm_portal *portal, struct qm_dqrr_entry *dq,
				int park)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	u8 idx = DQRR_PTR2IDX(dq);
	QM_ASSERT(dqrr->cmode == qm_dqrr_cdc);
	QM_ASSERT((dqrr->ring + idx) == dq);
	QM_ASSERT(idx < QM_DQRR_SIZE);
	qm_out(DQRR_DCAP, (0 << 8) |		/* DQRR_DCAP::S */
		((park ? 1 : 0) << 6) |		/* DQRR_DCAP::PK */
		idx);				/* DQRR_DCAP::DCAP_CI */
}

static inline void qm_dqrr_cdc_consume_n(struct qm_portal *portal, u16 bitmask)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cdc);
	qm_out(DQRR_DCAP, (1 << 8) |		/* DQRR_DCAP::S */
		((u32)bitmask << 16));		/* DQRR_DCAP::DCAP_CI */
}

static inline u8 qm_dqrr_cdc_cci(struct qm_portal *portal)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cdc);
	return qm_in(DQRR_CI_CINH) & (QM_DQRR_SIZE - 1);
}

static inline void qm_dqrr_cdc_cce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cdc);
	qm_cl_invalidate(DQRR_CI);
	qm_cl_touch_ro(DQRR_CI);
}

static inline u8 qm_dqrr_cdc_cce(struct qm_portal *portal)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode == qm_dqrr_cdc);
	return qm_cl_in(DQRR_CI) & (QM_DQRR_SIZE - 1);
}

static inline u8 qm_dqrr_get_ci(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode != qm_dqrr_cdc);
	return dqrr->ci;
}

static inline void qm_dqrr_park(struct qm_portal *portal, u8 idx)
{
	__maybe_unused register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode != qm_dqrr_cdc);
	qm_out(DQRR_DCAP, (0 << 8) |		/* S */
		(1 << 6) |			/* PK */
		(idx & (QM_DQRR_SIZE - 1)));	/* DCAP_CI */
}

static inline void qm_dqrr_park_ci(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	QM_ASSERT(dqrr->cmode != qm_dqrr_cdc);
	qm_out(DQRR_DCAP, (0 << 8) |		/* S */
		(1 << 6) |			/* PK */
		(dqrr->ci & (QM_DQRR_SIZE - 1)));/* DCAP_CI */
}

static inline void qm_dqrr_sdqcr_set(struct qm_portal *portal, u32 sdqcr)
{
	qm_out(DQRR_SDQCR, sdqcr);
}

static inline u32 qm_dqrr_sdqcr_get(struct qm_portal *portal)
{
	return qm_in(DQRR_SDQCR);
}

static inline void qm_dqrr_vdqcr_set(struct qm_portal *portal, u32 vdqcr)
{
	qm_out(DQRR_VDQCR, vdqcr);
}

static inline u32 qm_dqrr_vdqcr_get(struct qm_portal *portal)
{
	return qm_in(DQRR_VDQCR);
}

static inline void qm_dqrr_pdqcr_set(struct qm_portal *portal, u32 pdqcr)
{
	qm_out(DQRR_PDQCR, pdqcr);
}

static inline u32 qm_dqrr_pdqcr_get(struct qm_portal *portal)
{
	return qm_in(DQRR_PDQCR);
}

static inline u8 qm_dqrr_get_ithresh(struct qm_portal *portal)
{
	register struct qm_dqrr *dqrr = &portal->dqrr;
	return dqrr->ithresh;
}

static inline void qm_dqrr_set_ithresh(struct qm_portal *portal, u8 ithresh)
{
	qm_out(DQRR_ITR, ithresh);
}

static inline u8 qm_dqrr_get_maxfill(struct qm_portal *portal)
{
	return (qm_in(CFG) & 0x00f00000) >> 20;
}


/* -------------- */
/* --- MR API --- */

#define MR_CARRYCLEAR(p) \
	(void *)((unsigned long)(p) & (~(unsigned long)(QM_MR_SIZE << 6)))

static inline u8 MR_PTR2IDX(struct qm_mr_entry *e)
{
	return ((u32)e >> 6) & (QM_MR_SIZE - 1);
}

static inline struct qm_mr_entry *MR_INC(struct qm_mr_entry *e)
{
	return MR_CARRYCLEAR(e + 1);
}

static inline int qm_mr_init(struct qm_portal *portal, enum qm_mr_pmode pmode,
		enum qm_mr_cmode cmode)
{
	register struct qm_mr *mr = &portal->mr;
	u32 cfg;

#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1
	if ((qman_ip_rev == QMAN_REV1) && (pmode != qm_mr_pvb)) {
		pr_err("Qman is rev1, so QMAN9 workaround requires 'pvb'\n");
		return -EINVAL;
	}
#endif
	if (__qm_portal_bind(portal, QM_BIND_MR))
		return -EBUSY;
	mr->ring = ptr_OR(portal->addr.addr_ce, CL_MR);
	mr->pi = qm_in(MR_PI_CINH) & (QM_MR_SIZE - 1);
	mr->ci = qm_in(MR_CI_CINH) & (QM_MR_SIZE - 1);
#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1
	if (qman_ip_rev == QMAN_REV1)
		/* Situate the cursor in the shadow ring */
		mr->cursor = portal->bugs->mr + mr->ci;
	else
#endif
	mr->cursor = mr->ring + mr->ci;
	mr->fill = cyc_diff(QM_MR_SIZE, mr->ci, mr->pi);
	mr->vbit = (qm_in(MR_PI_CINH) & QM_MR_SIZE) ? QM_MR_VERB_VBIT : 0;
	mr->ithresh = qm_in(MR_ITR);
#ifdef CONFIG_FSL_QMAN_CHECKING
	mr->pmode = pmode;
	mr->cmode = cmode;
#endif
	cfg = (qm_in(CFG) & 0xfffff0ff) |
		((cmode & 1) << 8);		/* QCSP_CFG:MM */
	qm_out(CFG, cfg);
	return 0;
}

static inline void qm_mr_finish(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	if (mr->ci != MR_PTR2IDX(mr->cursor))
		pr_crit("Ignoring completed MR entries\n");
	__qm_portal_unbind(portal, QM_BIND_MR);
}

static inline void qm_mr_current_prefetch(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	dcbt_ro(mr->cursor);
}

static inline struct qm_mr_entry *qm_mr_current(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	if (!mr->fill)
		return NULL;
	return mr->cursor;
}

static inline u8 qm_mr_cursor(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	return MR_PTR2IDX(mr->cursor);
}

static inline u8 qm_mr_next(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->fill);
	mr->cursor = MR_INC(mr->cursor);
	return --mr->fill;
}

static inline u8 qm_mr_pci_update(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	u8 diff, old_pi = mr->pi;
	QM_ASSERT(mr->pmode == qm_mr_pci);
	mr->pi = qm_in(MR_PI_CINH);
	diff = cyc_diff(QM_MR_SIZE, old_pi, mr->pi);
	mr->fill += diff;
	return diff;
}

static inline void qm_mr_pce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->pmode == qm_mr_pce);
	qm_cl_invalidate(MR_PI);
	qm_cl_touch_ro(MR_PI);
}

static inline u8 qm_mr_pce_update(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	u8 diff, old_pi = mr->pi;
	QM_ASSERT(mr->pmode == qm_mr_pce);
	mr->pi = qm_cl_in(MR_PI) & (QM_MR_SIZE - 1);
	diff = cyc_diff(QM_MR_SIZE, old_pi, mr->pi);
	mr->fill += diff;
	return diff;
}

static inline void qm_mr_pvb_prefetch(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->pmode == qm_mr_pvb);
	dcbi(ptr_OR(mr->ring, qm_cl(mr->pi)));
	dcbt_ro(ptr_OR(mr->ring, qm_cl(mr->pi)));
}

static inline u8 qm_mr_pvb_update(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	struct qm_mr_entry *res = ptr_OR(mr->ring, qm_cl(mr->pi));
	QM_ASSERT(mr->pmode == qm_mr_pvb);
	if ((read_verb(&res->verb) & QM_MR_VERB_VBIT) == mr->vbit) {
#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1
		/* New MR entry, on affected chips, copy this to the shadow ring
		 * and fixup if required. */
		if (qman_ip_rev == QMAN_REV1) {
			struct qm_mr_entry *shadow = ptr_OR(portal->bugs->mr,
							qm_cl(mr->pi));
			copy_words(shadow, res, sizeof(*res));
			/* Bypass the QM_MR_RC_*** definitions, and check the
			 * byte value directly to handle the erratum. */
			if (shadow->ern.rc == 0x06)
				shadow->ern.rc = 0x60;
		}
#endif
		mr->pi = (mr->pi + 1) & (QM_MR_SIZE - 1);
		if (!mr->pi)
			mr->vbit ^= QM_MR_VERB_VBIT;
		mr->fill++;
		return 1;
	}
	return 0;
}

static inline void qm_mr_cci_consume(struct qm_portal *portal, u8 num)
{
	register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->cmode == qm_mr_cci);
	mr->ci = (mr->ci + num) & (QM_MR_SIZE - 1);
	qm_out(MR_CI_CINH, mr->ci);
}

static inline void qm_mr_cci_consume_to_current(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->cmode == qm_mr_cci);
	mr->ci = MR_PTR2IDX(mr->cursor);
	qm_out(MR_CI_CINH, mr->ci);
}

static inline void qm_mr_cce_prefetch(struct qm_portal *portal)
{
	__maybe_unused register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->cmode == qm_mr_cce);
	qm_cl_invalidate(MR_CI);
	qm_cl_touch_rw(MR_CI);
}

static inline void qm_mr_cce_consume(struct qm_portal *portal, u8 num)
{
	register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->cmode == qm_mr_cce);
	mr->ci = (mr->ci + num) & (QM_MR_SIZE - 1);
	qm_cl_out(MR_CI, mr->ci);
}

static inline void qm_mr_cce_consume_to_current(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	QM_ASSERT(mr->cmode == qm_mr_cce);
	mr->ci = MR_PTR2IDX(mr->cursor);
	qm_cl_out(MR_CI, mr->ci);
}

static inline u8 qm_mr_get_ci(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	return mr->ci;
}

static inline u8 qm_mr_get_ithresh(struct qm_portal *portal)
{
	register struct qm_mr *mr = &portal->mr;
	return mr->ithresh;
}

static inline void qm_mr_set_ithresh(struct qm_portal *portal, u8 ithresh)
{
	qm_out(MR_ITR, ithresh);
}


/* ------------------------------ */
/* --- Management command API --- */

static inline int qm_mc_init(struct qm_portal *portal)
{
	register struct qm_mc *mc = &portal->mc;
	if (__qm_portal_bind(portal, QM_BIND_MC))
		return -EBUSY;
	mc->cr = ptr_OR(portal->addr.addr_ce, CL_CR);
	mc->rr = ptr_OR(portal->addr.addr_ce, CL_RR0);
	mc->rridx = (read_verb(&mc->cr->__dont_write_directly__verb) &
			QM_MCC_VERB_VBIT) ?  0 : 1;
	mc->vbit = mc->rridx ? QM_MCC_VERB_VBIT : 0;
#ifdef CONFIG_FSL_QMAN_CHECKING
	mc->state = mc_idle;
#endif
	return 0;
}

static inline void qm_mc_finish(struct qm_portal *portal)
{
	__maybe_unused register struct qm_mc *mc = &portal->mc;
	QM_ASSERT(mc->state == mc_idle);
#ifdef CONFIG_FSL_QMAN_CHECKING
	if (mc->state != mc_idle)
		pr_crit("Losing incomplete MC command\n");
#endif
	__qm_portal_unbind(portal, QM_BIND_MC);
}
 
static inline struct qm_mc_command *qm_mc_start(struct qm_portal *portal)
{
	register struct qm_mc *mc = &portal->mc;
	QM_ASSERT(mc->state == mc_idle);
#ifdef CONFIG_FSL_QMAN_CHECKING
	mc->state = mc_user;
#endif
	dcbzl(mc->cr);
	return mc->cr;
}

static inline void qm_mc_abort(struct qm_portal *portal)
{
	__maybe_unused register struct qm_mc *mc = &portal->mc;
	QM_ASSERT(mc->state == mc_user);
#ifdef CONFIG_FSL_QMAN_CHECKING
	mc->state = mc_idle;
#endif
}

static inline void qm_mc_commit(struct qm_portal *portal, u8 myverb)
{
	register struct qm_mc *mc = &portal->mc;
	struct qm_mc_result *rr = mc->rr + mc->rridx;
	QM_ASSERT(mc->state == mc_user);
	dcbi(rr);
	lwsync();
	//printf("loading qm_mc_commit--1!\n");
#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1
   // printf("loading qm_mc_commit--2!\n");
	if ((qman_ip_rev == QMAN_REV1) && ((myverb & QM_MCC_VERB_MASK) ==
					QM_MCC_VERB_INITFQ_SCHED)) 
	{
	/* ffffffffffffffffffffffffffffffffffffff */
		u32 fqid = mc->cr->initfq.fqid;
		/* Do two commands to avoid the hw bug. Note, we poll locally
		 * rather than using qm_mc_result() because from a QMAN_CHECKING
		 * perspective, we don't want to appear to have "finished" until
		 * both commands are done. */
		// printf("loading qm_mc_commit--3!\n");
		mc->cr->__dont_write_directly__verb = mc->vbit |
					QM_MCC_VERB_INITFQ_PARKED;
		dcbf(mc->cr);
		portal->bugs->initfq_and_sched = 1;
		//printf("loading qm_mc_commit!\n");
	//	#if 1
		do {
			dcbi(rr);
			dcbt_ro(rr);
			barrier();
		} while (!read_verb(&rr->verb));
	//	#endif
		
#ifdef CONFIG_FSL_QMAN_CHECKING
		mc->state = mc_idle;
#endif
		if (rr->result != QM_MCR_RESULT_OK) {
#ifdef CONFIG_FSL_QMAN_CHECKING
			mc->state = mc_hw;
#endif
			return;
		}
		mc->rridx ^= 1;
		mc->vbit ^= QM_MCC_VERB_VBIT;
		rr = mc->rr + mc->rridx;
		dcbzl(mc->cr);
		mc->cr->alterfq.fqid = fqid;
		dcbi(rr);
		lwsync();
		myverb = QM_MCC_VERB_ALTER_SCHED;
	} else
		portal->bugs->initfq_and_sched = 0;
#endif
	mc->cr->__dont_write_directly__verb = myverb | mc->vbit;
	dcbf(mc->cr);
	dcbt_ro(rr);    
#ifdef CONFIG_FSL_QMAN_CHECKING
	mc->state = mc_hw;
#endif
}

static inline struct qm_mc_result *qm_mc_result(struct qm_portal *portal)
{
    register struct qm_mc *mc = &portal->mc;
    struct qm_mc_result *rr = mc->rr + mc->rridx;
	  QM_ASSERT(mc->state == mc_hw);
	  /* The inactive response register's verb byte always returns zero until
	   * its command is submitted and completed. This includes the valid-bit,
	   * in case you were wondering... */
	if (!read_verb(&rr->verb)) {
		dcbi(rr);
		dcbt_ro(rr);
		return NULL;
	}
#ifdef CONFIG_FSL_QMAN_BUG_AND_FEATURE_REV1
	if (qman_ip_rev == QMAN_REV1) {
		if ((read_verb(&rr->verb) & QM_MCR_VERB_MASK) ==
						QM_MCR_VERB_QUERYFQ) {
			void *misplaced = (void *)rr + 50;
			copy_words(&portal->bugs->result, rr, sizeof(*rr));
			rr = &portal->bugs->result;
			copy_shorts(&rr->queryfq.fqd.td, misplaced,
				sizeof(rr->queryfq.fqd.td));
		} else if (portal->bugs->initfq_and_sched) {
			/* We split the user-requested command, make the final
			 * result match the requested type. */
			copy_words(&portal->bugs->result, rr, sizeof(*rr));
			rr = &portal->bugs->result;
			rr->verb = (rr->verb & QM_MCR_VERB_RRID) |
					QM_MCR_VERB_INITFQ_SCHED;
		}
	}
#endif
	mc->rridx ^= 1;
	mc->vbit ^= QM_MCC_VERB_VBIT;
#ifdef CONFIG_FSL_QMAN_CHECKING
	mc->state = mc_idle;
#endif
	return rr;
}


/* ------------------------------------- */
/* --- Portal interrupt register API --- */

static inline int qm_isr_init(struct qm_portal *portal)
{
	if (__qm_portal_bind(portal, QM_BIND_ISR))
		return -EBUSY;
	return 0;
}

static inline void qm_isr_finish(struct qm_portal *portal)
{
	__qm_portal_unbind(portal, QM_BIND_ISR);
}

static inline void qm_isr_set_iperiod(struct qm_portal *portal, u16 iperiod)
{
	qm_out(ITPR, iperiod);
}

static inline u32 __qm_isr_read(struct qm_portal *portal, enum qm_isr_reg n)
{
	return __qm_in(&portal->addr, REG_ISR + (n << 2));
}

static inline void __qm_isr_write(struct qm_portal *portal, enum qm_isr_reg n, u32 val)
{
	__qm_out(&portal->addr, REG_ISR + (n << 2), val);
}

